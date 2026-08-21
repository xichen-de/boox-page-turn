#include "ble_hid.h"

#include <string.h>

#include "config.h"
#include "ui.h"

#include "esp_bt.h"
#include "esp_err.h"
#include "esp_hid_common.h"
#include "esp_hidd.h"
#include "esp_log.h"
#include "esp_timer.h"
#include "host/ble_gap.h"
#include "host/ble_hs.h"
#include "host/ble_hs_adv.h"
#include "host/ble_store.h"
#include "host/ble_uuid.h"
#include "nimble/ble.h"
#include "nimble/nimble_port.h"
#include "nimble/nimble_port_freertos.h"
#include "services/gap/ble_svc_gap.h"

#define HID_REPORT_ID 1
#define KEY_RELEASE_DELAY_US 12000

static const char *TAG = "boox_ble_hid";

/* Standard boot-style keyboard report: modifiers, reserved byte, six keys. */
static const uint8_t s_keyboard_report_map[] = {
    0x05, 0x01,       /* Usage Page (Generic Desktop) */
    0x09, 0x06,       /* Usage (Keyboard) */
    0xa1, 0x01,       /* Collection (Application) */
    0x85, HID_REPORT_ID,
    0x05, 0x07,       /* Usage Page (Keyboard) */
    0x19, 0xe0,
    0x29, 0xe7,
    0x15, 0x00,
    0x25, 0x01,
    0x75, 0x01,
    0x95, 0x08,
    0x81, 0x02,       /* Modifier input */
    0x95, 0x01,
    0x75, 0x08,
    0x81, 0x03,       /* Reserved */
    0x95, 0x05,
    0x75, 0x01,
    0x05, 0x08,
    0x19, 0x01,
    0x29, 0x05,
    0x91, 0x02,       /* Keyboard LED output */
    0x95, 0x01,
    0x75, 0x03,
    0x91, 0x03,
    0x95, 0x06,
    0x75, 0x08,
    0x15, 0x00,
    0x25, 0x65,
    0x05, 0x07,
    0x19, 0x00,
    0x29, 0x65,
    0x81, 0x00,       /* Six-key rollover input */
    0xc0,
};

static esp_hid_raw_report_map_t s_report_maps[] = {
    {
        .data = s_keyboard_report_map,
        .len = sizeof(s_keyboard_report_map),
    },
};

static esp_hid_device_config_t s_hid_config = {
    .vendor_id = 0x303a,
    .product_id = 0x4001,
    .version = 0x0100,
    .device_name = DEVICE_NAME,
    .manufacturer_name = "M5Stack",
    .serial_number = "CoreS3",
    .report_maps = s_report_maps,
    .report_maps_len = 1,
};

static esp_hidd_dev_t *s_hid_device;
static volatile bool s_connected;
static bool s_hid_started;
static bool s_ble_synced;
static esp_timer_handle_t s_key_release_timer;

static ble_uuid16_t s_hid_service_uuid = BLE_UUID16_INIT(0x1812);
static struct ble_hs_adv_fields s_adv_fields;
static uint8_t s_own_addr_type;

static void start_advertising(void);

static void send_report(uint8_t keycode)
{
    uint8_t report[8] = {0};
    report[2] = keycode;
    ESP_ERROR_CHECK_WITHOUT_ABORT(
        esp_hidd_dev_input_set(s_hid_device, 0, HID_REPORT_ID, report, sizeof(report)));
}

static void key_release_timer_cb(void *arg)
{
    (void)arg;
    send_report(0);
}

void ble_hid_send_arrow(uint8_t keycode)
{
    if (!s_connected || s_hid_device == NULL) {
        ESP_LOGW(TAG, "Tap ignored while BLE is disconnected");
        return;
    }

    /* If a previous key's release is still pending, fire it immediately so
     * the host always sees a key-up before the next key-down, then queue
     * this key's release without blocking the caller (the LVGL UI task). */
    if (esp_timer_is_active(s_key_release_timer)) {
        ESP_ERROR_CHECK_WITHOUT_ABORT(esp_timer_stop(s_key_release_timer));
        send_report(0);
    }

    send_report(keycode);
    ESP_ERROR_CHECK_WITHOUT_ABORT(
        esp_timer_start_once(s_key_release_timer, KEY_RELEASE_DELAY_US));
}

static int gap_event_cb(struct ble_gap_event *event, void *arg)
{
    (void)arg;
    switch (event->type) {
    case BLE_GAP_EVENT_CONNECT:
        if (event->connect.status != 0) {
            start_advertising();
        }
        break;
    case BLE_GAP_EVENT_DISCONNECT:
    case BLE_GAP_EVENT_ADV_COMPLETE:
        start_advertising();
        break;
    case BLE_GAP_EVENT_REPEAT_PAIRING: {
        struct ble_gap_conn_desc desc;
        if (ble_gap_conn_find(event->repeat_pairing.conn_handle, &desc) == 0) {
            ble_store_util_delete_peer(&desc.peer_id_addr);
        }
        return BLE_GAP_REPEAT_PAIRING_RETRY;
    }
    default:
        break;
    }
    return 0;
}

static void start_advertising(void)
{
    if (!s_ble_synced || !s_hid_started || ble_gap_adv_active()) {
        return;
    }

    struct ble_gap_adv_params params = {0};
    params.conn_mode = BLE_GAP_CONN_MODE_UND;
    params.disc_mode = BLE_GAP_DISC_MODE_GEN;
    /* A page remote can trade a sub-second reconnect delay for substantially
     * less radio activity while its reader is unavailable. */
    params.itvl_min = BLE_GAP_ADV_ITVL_MS(250);
    params.itvl_max = BLE_GAP_ADV_ITVL_MS(500);

    int rc = ble_gap_adv_set_fields(&s_adv_fields);
    if (rc == 0) {
        rc = ble_gap_adv_start(s_own_addr_type, NULL, BLE_HS_FOREVER,
                               &params, gap_event_cb, NULL);
    }
    if (rc != 0) {
        ESP_LOGE(TAG, "Advertising failed: %d", rc);
    }
}

static void hid_event_cb(void *handler_args, esp_event_base_t base,
                         int32_t id, void *event_data)
{
    (void)handler_args;
    (void)base;
    (void)event_data;

    switch ((esp_hidd_event_t)id) {
    case ESP_HIDD_START_EVENT: {
        int rc = ble_hs_id_infer_auto(0, &s_own_addr_type);
        if (rc != 0) {
            ESP_LOGE(TAG, "Could not determine BLE address type: %d", rc);
            break;
        }
        s_ble_synced = true;
        s_hid_started = true;
        start_advertising();
        break;
    }
    case ESP_HIDD_CONNECT_EVENT:
        ESP_LOGI(TAG, "BOOX connected");
        s_connected = true;
        ui_notify_interaction();
        ui_set_connected(true);
        break;
    case ESP_HIDD_DISCONNECT_EVENT:
        ESP_LOGI(TAG, "BOOX disconnected");
        s_connected = false;
        ui_set_connected(false);
        break;
    default:
        break;
    }
}

static void nimble_host_task(void *param)
{
    (void)param;
    nimble_port_run();
    nimble_port_freertos_deinit();
}

void ble_store_config_init(void);

void ble_hid_init(void)
{
    const esp_timer_create_args_t release_timer_args = {
        .callback = key_release_timer_cb,
        .name = "hid_key_release",
    };
    ESP_ERROR_CHECK(esp_timer_create(&release_timer_args, &s_key_release_timer));

    esp_bt_controller_config_t bt_config = BT_CONTROLLER_INIT_CONFIG_DEFAULT();
    ESP_ERROR_CHECK(esp_bt_controller_init(&bt_config));
    ESP_ERROR_CHECK(esp_bt_controller_enable(ESP_BT_MODE_BLE));
    ESP_ERROR_CHECK(esp_nimble_init());

    memset(&s_adv_fields, 0, sizeof(s_adv_fields));
    s_adv_fields.flags = BLE_HS_ADV_F_DISC_GEN | BLE_HS_ADV_F_BREDR_UNSUP;
    s_adv_fields.appearance = ESP_HID_APPEARANCE_KEYBOARD;
    s_adv_fields.appearance_is_present = 1;
    s_adv_fields.tx_pwr_lvl = BLE_HS_ADV_TX_PWR_LVL_AUTO;
    s_adv_fields.tx_pwr_lvl_is_present = 1;
    s_adv_fields.name = (uint8_t *)DEVICE_NAME;
    s_adv_fields.name_len = strlen(DEVICE_NAME);
    s_adv_fields.name_is_complete = 1;
    s_adv_fields.uuids16 = &s_hid_service_uuid;
    s_adv_fields.num_uuids16 = 1;
    s_adv_fields.uuids16_is_complete = 1;

    /* BOOX has no reason to type a PIN into this two-button peripheral. */
    ble_hs_cfg.sm_io_cap = BLE_SM_IO_CAP_NO_IO;
    ble_hs_cfg.sm_bonding = 1;
    ble_hs_cfg.sm_mitm = 0;
    ble_hs_cfg.sm_sc = 1;
    ble_hs_cfg.sm_our_key_dist = BLE_SM_PAIR_KEY_DIST_ENC | BLE_SM_PAIR_KEY_DIST_ID;
    ble_hs_cfg.sm_their_key_dist = BLE_SM_PAIR_KEY_DIST_ENC | BLE_SM_PAIR_KEY_DIST_ID;

    ble_store_config_init();
    ESP_ERROR_CHECK(esp_hidd_dev_init(&s_hid_config, ESP_HID_TRANSPORT_BLE,
                                      hid_event_cb, &s_hid_device));
    ESP_ERROR_CHECK(ble_svc_gap_device_name_set(DEVICE_NAME));
    ESP_ERROR_CHECK(esp_nimble_enable(nimble_host_task));
}

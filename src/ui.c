#include "ui.h"

#include "ble_hid.h"
#include "config.h"
#include "idle_policy.h"
#include "power.h"
#include "ui_screen.h"

#include "bsp/display.h"
#include "bsp/m5stack_core_s3.h"
#include "esp_err.h"
#include "esp_timer.h"

#define NORMAL_BRIGHTNESS 25
#define DIM_BRIGHTNESS 5
#define BATTERY_POLL_INTERVAL_MS (60UL * 1000UL)
#define LOW_BATTERY_PERCENT 20

typedef enum {
    DISPLAY_STATE_ACTIVE,
    DISPLAY_STATE_DIMMED,
    DISPLAY_STATE_OFF,
} display_state_t;

static ui_screen_t s_screen;
static display_state_t s_display_state = DISPLAY_STATE_OFF;
static uint32_t s_last_interaction_ms;
static bool s_shutdown_requested;

static uint32_t now_ms(void)
{
    return (uint32_t)(esp_timer_get_time() / 1000);
}

void ui_notify_interaction(void)
{
    s_last_interaction_ms = now_ms();
    s_shutdown_requested = false;
}

static void set_display_state(display_state_t state)
{
    if (state == s_display_state) {
        return;
    }

    int brightness = NORMAL_BRIGHTNESS;
    if (state == DISPLAY_STATE_DIMMED) {
        brightness = DIM_BRIGHTNESS;
    } else if (state == DISPLAY_STATE_OFF) {
        brightness = 0;
    }

    ESP_ERROR_CHECK_WITHOUT_ABORT(bsp_display_brightness_set(brightness));
    s_display_state = state;
}

void ui_set_connected(bool connected)
{
    if (s_screen.status_dot == NULL || !bsp_display_lock(50)) {
        return;
    }

    ui_screen_set_connected(&s_screen, connected);
    bsp_display_unlock();
}

static void update_battery_status(void)
{
    if (s_screen.battery_label == NULL) {
        return;
    }

    bool battery_present;
    bool charging;
    uint8_t percent;
    if (!power_query(&battery_present, &charging, &percent)) {
        return;
    }

    bool low = battery_present && percent <= LOW_BATTERY_PERCENT;
    const char *text = low ? (charging ? "LOW+" : "LOW") : (charging ? "CHG" : "");

    ui_screen_set_battery_text(&s_screen, text);
}

static void battery_timer_cb(lv_timer_t *timer)
{
    (void)timer;
    update_battery_status();
}

static void button_event_cb(lv_event_t *event)
{
    lv_event_code_t code = lv_event_get_code(event);

    if (code == LV_EVENT_PRESSED) {
        ui_notify_interaction();
        set_display_state(DISPLAY_STATE_ACTIVE);

        uintptr_t side = (uintptr_t)lv_event_get_user_data(event);
        ble_hid_send_arrow(side == 0 ? HID_KEY_LEFT_ARROW : HID_KEY_RIGHT_ARROW);
    } else if (code == LV_EVENT_RELEASED || code == LV_EVENT_PRESS_LOST) {
        ui_notify_interaction();
    }
}

static void display_timeout_cb(lv_timer_t *timer)
{
    (void)timer;
    uint32_t idle_ms = idle_elapsed_ms(now_ms(), s_last_interaction_ms);
    idle_action_t action = idle_action_for_elapsed(idle_ms);

    if (action == IDLE_ACTION_POWER_OFF) {
        if (!s_shutdown_requested) {
            s_shutdown_requested = power_shutdown() == ESP_OK;
        }
    } else if (action == IDLE_ACTION_TURN_DISPLAY_OFF) {
        set_display_state(DISPLAY_STATE_OFF);
    } else if (action == IDLE_ACTION_DIM_DISPLAY) {
        set_display_state(DISPLAY_STATE_DIMMED);
    }
}

void ui_init(void)
{
    lv_display_t *display = bsp_display_start();
    ESP_ERROR_CHECK(display == NULL ? ESP_FAIL : ESP_OK);
    set_display_state(DISPLAY_STATE_ACTIVE);

    ESP_ERROR_CHECK_WITHOUT_ABORT(power_init(bsp_i2c_get_handle()));

    bsp_display_lock(0);
    ui_screen_create(&s_screen, lv_display_get_screen_active(display),
                     button_event_cb);

    update_battery_status();
    lv_timer_create(display_timeout_cb, 1000, NULL);
    lv_timer_create(battery_timer_cb, BATTERY_POLL_INTERVAL_MS, NULL);
    bsp_display_unlock();

    ui_notify_interaction();
}

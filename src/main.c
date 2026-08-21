/*
 * Minimal M5Stack CoreS3 page-turn remote for BOOX readers.
 * PlatformIO + native ESP-IDF (no Arduino compatibility layer).
 */

#include "esp_err.h"
#include "esp_log.h"
#include "esp_pm.h"
#include "nvs_flash.h"

#include "ble_hid.h"
#include "config.h"
#include "ui.h"

static const char *TAG = "boox_remote";

static void power_management_init(void)
{
    const esp_pm_config_t config = {
        .max_freq_mhz = CONFIG_ESP_DEFAULT_CPU_FREQ_MHZ,
        .min_freq_mhz = 40,
        /* The ESP32-S3 Bluetooth controller holds a light-sleep lock while
         * enabled. BLE modem sleep still powers down the radio between events. */
        .light_sleep_enable = false,
    };
    ESP_ERROR_CHECK(esp_pm_configure(&config));
}

void app_main(void)
{
    esp_err_t err = nvs_flash_init();
    if (err == ESP_ERR_NVS_NO_FREE_PAGES || err == ESP_ERR_NVS_NEW_VERSION_FOUND) {
        ESP_ERROR_CHECK(nvs_flash_erase());
        err = nvs_flash_init();
    }
    ESP_ERROR_CHECK(err);

    power_management_init();
    ui_init();
    ble_hid_init();
    ESP_LOGI(TAG, "%s ready", DEVICE_NAME);
}

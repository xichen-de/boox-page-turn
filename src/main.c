/*
 * Minimal M5Stack CoreS3 page-turn remote for BOOX readers.
 * PlatformIO + native ESP-IDF (no Arduino compatibility layer).
 */

#include "esp_err.h"
#include "esp_log.h"
#include "nvs_flash.h"

#include "ble_hid.h"
#include "config.h"
#include "ui.h"

static const char *TAG = "boox_remote";

void app_main(void)
{
    esp_err_t err = nvs_flash_init();
    if (err == ESP_ERR_NVS_NO_FREE_PAGES || err == ESP_ERR_NVS_NEW_VERSION_FOUND) {
        ESP_ERROR_CHECK(nvs_flash_erase());
        err = nvs_flash_init();
    }
    ESP_ERROR_CHECK(err);

    ui_init();
    ble_hid_init();
    ESP_LOGI(TAG, "%s ready", DEVICE_NAME);
}

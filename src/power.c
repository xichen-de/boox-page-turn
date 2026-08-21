#include "power.h"

#include "esp_log.h"

#define AXP2101_I2C_ADDRESS 0x34
#define AXP2101_STATUS1_REGISTER 0x00
#define AXP2101_STATUS2_REGISTER 0x01
#define AXP2101_COMMON_CONFIG_REGISTER 0x10
#define AXP2101_BATTERY_PERCENT_REGISTER 0xa4
#define AXP2101_BATTERY_PRESENT_MASK 0x08
#define AXP2101_CHARGE_STATE_MASK 0x60
#define AXP2101_CHARGING_STATE 0x20
#define AXP2101_SOFT_POWER_OFF_MASK 0x01
#define I2C_TIMEOUT_MS 100

static const char *TAG = "boox_power";

static i2c_master_dev_handle_t s_power_device;

esp_err_t power_init(i2c_master_bus_handle_t bus)
{
    const i2c_device_config_t power_config = {
        .dev_addr_length = I2C_ADDR_BIT_LEN_7,
        .device_address = AXP2101_I2C_ADDRESS,
        .scl_speed_hz = 400000,
    };
    return i2c_master_bus_add_device(bus, &power_config, &s_power_device);
}

static esp_err_t read_register(uint8_t address, uint8_t *value)
{
    return i2c_master_transmit_receive(s_power_device, &address, 1, value, 1,
                                       I2C_TIMEOUT_MS);
}

static esp_err_t write_register(uint8_t address, uint8_t value)
{
    uint8_t data[] = {address, value};
    return i2c_master_transmit(s_power_device, data, sizeof(data),
                               I2C_TIMEOUT_MS);
}

bool power_query(bool *battery_present, bool *charging, uint8_t *percent)
{
    if (s_power_device == NULL) {
        return false;
    }

    uint8_t status1;
    uint8_t status2;
    if (read_register(AXP2101_STATUS1_REGISTER, &status1) != ESP_OK ||
            read_register(AXP2101_STATUS2_REGISTER, &status2) != ESP_OK ||
            read_register(AXP2101_BATTERY_PERCENT_REGISTER, percent) != ESP_OK) {
        ESP_LOGW(TAG, "Could not read battery status");
        return false;
    }

    *battery_present = (status1 & AXP2101_BATTERY_PRESENT_MASK) != 0;
    *charging = (status2 & AXP2101_CHARGE_STATE_MASK) == AXP2101_CHARGING_STATE;
    return true;
}

esp_err_t power_shutdown(void)
{
    if (s_power_device == NULL) {
        return ESP_ERR_INVALID_STATE;
    }

    uint8_t config;
    esp_err_t err = read_register(AXP2101_COMMON_CONFIG_REGISTER, &config);
    if (err != ESP_OK) {
        ESP_LOGE(TAG, "Could not read PMIC shutdown register: %s",
                 esp_err_to_name(err));
        return err;
    }

    ESP_LOGI(TAG, "Powering off after idle timeout");
    err = write_register(AXP2101_COMMON_CONFIG_REGISTER,
                         config | AXP2101_SOFT_POWER_OFF_MASK);
    if (err != ESP_OK) {
        ESP_LOGE(TAG, "Could not request PMIC shutdown: %s",
                 esp_err_to_name(err));
    }
    return err;
}

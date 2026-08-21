#pragma once

#include <stdbool.h>
#include <stdint.h>

#include "driver/i2c_master.h"
#include "esp_err.h"

/* Adds the AXP2101 PMIC as an I2C device on the given bus. */
esp_err_t power_init(i2c_master_bus_handle_t bus);

/* Reads current battery state. Returns false (and logs) if the I2C reads failed. */
bool power_query(bool *battery_present, bool *charging, uint8_t *percent);

/* Requests a complete shutdown from the AXP2101 PMIC. The side power button
 * turns the CoreS3 on again. */
esp_err_t power_shutdown(void);

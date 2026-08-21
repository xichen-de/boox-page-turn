#pragma once

#include <stdint.h>

#define HID_KEY_RIGHT_ARROW 0x4f
#define HID_KEY_LEFT_ARROW 0x50

/* Brings up the BT controller, NimBLE stack, and BLE HID keyboard service.
 * Advertising starts automatically once the HID service is ready. */
void ble_hid_init(void);

/* Sends a key-down report for `keycode`, then schedules the key-up report a
 * short time later without blocking the caller. No-op if not connected. */
void ble_hid_send_arrow(uint8_t keycode);

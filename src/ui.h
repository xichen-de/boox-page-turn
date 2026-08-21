#pragma once

#include <stdbool.h>

/* Starts the display, builds the button screen, and arms the dimming and
 * battery-poll timers. Must run before ble_hid_init() reports connection
 * state changes. */
void ui_init(void);

/* Updates the connection status dot. Safe to call from any task. */
void ui_set_connected(bool connected);

/* Resets the idle timer so the display stays bright / wakes up. */
void ui_notify_interaction(void);

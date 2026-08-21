#pragma once

#include <stdbool.h>

#include "lvgl.h"

typedef struct {
    lv_obj_t *status_dot;
    lv_obj_t *battery_label;
} ui_screen_t;

/* Builds the shared device/preview UI. event_cb may be NULL for a static render. */
void ui_screen_create(ui_screen_t *ui, lv_obj_t *screen, lv_event_cb_t event_cb);

void ui_screen_set_connected(ui_screen_t *ui, bool connected);
void ui_screen_set_battery_text(ui_screen_t *ui, const char *text);

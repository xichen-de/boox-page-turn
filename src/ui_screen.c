#include "ui_screen.h"

#include <stdint.h>
#include <string.h>

#include "config.h"

static void style_status_dot(lv_obj_t *dot, bool connected)
{
    lv_obj_set_style_border_width(dot, connected ? 0 : 1, LV_PART_MAIN);
    lv_obj_set_style_border_color(dot, lv_color_hex(0x657080), LV_PART_MAIN);
    lv_obj_set_style_bg_opa(dot, connected ? LV_OPA_COVER : LV_OPA_TRANSP,
                            LV_PART_MAIN);
    lv_obj_set_style_bg_color(dot, lv_color_hex(0x39a06b), LV_PART_MAIN);
}

static lv_obj_t *make_button(lv_obj_t *screen, bool right,
                             lv_event_cb_t event_cb)
{
    lv_obj_t *button = lv_button_create(screen);
    lv_obj_set_size(button, LV_PCT(50), LV_PCT(100));
    lv_obj_align(button, right ? LV_ALIGN_RIGHT_MID : LV_ALIGN_LEFT_MID, 0, 0);
    lv_obj_set_style_radius(button, 0, LV_PART_MAIN);
    lv_obj_set_style_border_width(button, 0, LV_PART_MAIN);
    lv_obj_set_style_shadow_width(button, 0, LV_PART_MAIN);
    lv_obj_set_style_bg_color(button, lv_color_hex(0x101318), LV_PART_MAIN);
    lv_obj_set_style_bg_color(button, lv_color_hex(0x29313b),
                              LV_PART_MAIN | LV_STATE_PRESSED);
    lv_obj_set_style_text_color(button, lv_color_hex(0xa8b0ba), LV_PART_MAIN);
    lv_obj_set_style_text_color(button, lv_color_hex(0xc2c8cf),
                                LV_PART_MAIN | LV_STATE_PRESSED);
    if (event_cb != NULL) {
        lv_obj_add_event_cb(button, event_cb, LV_EVENT_ALL,
                            (void *)(uintptr_t)right);
    }

    lv_obj_t *arrow = lv_label_create(button);
    lv_label_set_text(arrow, right ? ">" : "<");
    lv_obj_set_style_text_font(arrow, &lv_font_montserrat_48, LV_PART_MAIN);
    lv_obj_align(arrow, LV_ALIGN_CENTER, 0, -16);

    lv_obj_t *caption = lv_label_create(button);
    lv_label_set_text(caption, right ? "NEXT" : "PREV");
    lv_obj_set_style_text_font(caption, &lv_font_montserrat_14, LV_PART_MAIN);
    lv_obj_align(caption, LV_ALIGN_CENTER, 0, 38);
    return button;
}

void ui_screen_create(ui_screen_t *ui, lv_obj_t *screen,
                      lv_event_cb_t event_cb)
{
    lv_obj_set_style_bg_color(screen, lv_color_hex(0x101318), LV_PART_MAIN);
    lv_obj_clear_flag(screen, LV_OBJ_FLAG_SCROLLABLE);

    lv_obj_t *left = make_button(screen, false, event_cb);
    lv_obj_t *right = make_button(screen, true, event_cb);

    lv_obj_t *title = lv_label_create(left);
    lv_label_set_text(title, DEVICE_NAME);
    lv_obj_set_style_text_color(title, lv_color_hex(0x9aa6b5), LV_PART_MAIN);
    lv_obj_align(title, LV_ALIGN_TOP_LEFT, 8, 6);

    ui->status_dot = lv_obj_create(right);
    lv_obj_remove_style_all(ui->status_dot);
    lv_obj_set_size(ui->status_dot, 9, 9);
    lv_obj_set_style_radius(ui->status_dot, LV_RADIUS_CIRCLE, LV_PART_MAIN);
    style_status_dot(ui->status_dot, false);
    lv_obj_clear_flag(ui->status_dot, LV_OBJ_FLAG_CLICKABLE);
    lv_obj_align(ui->status_dot, LV_ALIGN_TOP_RIGHT, -9, 9);

    ui->battery_label = lv_label_create(right);
    lv_label_set_text(ui->battery_label, "");
    lv_obj_set_style_text_font(ui->battery_label, &lv_font_montserrat_14,
                               LV_PART_MAIN);
    lv_obj_set_style_text_color(ui->battery_label, lv_color_hex(0xa9824f),
                                LV_PART_MAIN);
    lv_obj_align(ui->battery_label, LV_ALIGN_TOP_RIGHT, -25, 5);
}

void ui_screen_set_connected(ui_screen_t *ui, bool connected)
{
    if (ui->status_dot != NULL) {
        style_status_dot(ui->status_dot, connected);
    }
}

void ui_screen_set_battery_text(ui_screen_t *ui, const char *text)
{
    if (ui->battery_label != NULL &&
            strcmp(lv_label_get_text(ui->battery_label), text) != 0) {
        lv_label_set_text(ui->battery_label, text);
    }
}

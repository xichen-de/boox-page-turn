#include "ui_screen.h"

#include <stdint.h>
#include <string.h>

#include "config.h"

#define COLOR_BACKGROUND 0x0d1117
#define COLOR_SURFACE 0x171d25
#define COLOR_SURFACE_PRESSED 0x252e3a
#define COLOR_BORDER 0x303a47
#define COLOR_TEXT 0xdce3ec
#define COLOR_TEXT_MUTED 0x8f9baa
#define COLOR_ACCENT 0x77b7ff
#define COLOR_CONNECTED 0x42b883
#define COLOR_DISCONNECTED 0x657080

static void style_status_dot(lv_obj_t *dot, bool connected)
{
    lv_obj_set_style_border_width(dot, connected ? 0 : 1, LV_PART_MAIN);
    lv_obj_set_style_border_color(dot, lv_color_hex(COLOR_DISCONNECTED),
                                  LV_PART_MAIN);
    lv_obj_set_style_bg_opa(dot, connected ? LV_OPA_COVER : LV_OPA_TRANSP,
                            LV_PART_MAIN);
    lv_obj_set_style_bg_color(dot, lv_color_hex(COLOR_CONNECTED), LV_PART_MAIN);
}

static lv_obj_t *make_button(lv_obj_t *screen, bool right,
                             lv_event_cb_t event_cb)
{
    lv_obj_t *button = lv_button_create(screen);
    lv_obj_set_size(button, 148, 174);
    lv_obj_align(button, right ? LV_ALIGN_BOTTOM_RIGHT : LV_ALIGN_BOTTOM_LEFT,
                 right ? -8 : 8, -8);
    lv_obj_set_style_radius(button, 18, LV_PART_MAIN);
    lv_obj_set_style_border_width(button, 1, LV_PART_MAIN);
    lv_obj_set_style_border_color(button, lv_color_hex(COLOR_BORDER),
                                  LV_PART_MAIN);
    lv_obj_set_style_border_color(button, lv_color_hex(COLOR_ACCENT),
                                  LV_PART_MAIN | LV_STATE_PRESSED);
    lv_obj_set_style_shadow_width(button, 0, LV_PART_MAIN);
    lv_obj_set_style_bg_color(button, lv_color_hex(COLOR_SURFACE), LV_PART_MAIN);
    lv_obj_set_style_bg_color(button, lv_color_hex(COLOR_SURFACE_PRESSED),
                              LV_PART_MAIN | LV_STATE_PRESSED);
    lv_obj_set_style_text_color(button, lv_color_hex(COLOR_TEXT), LV_PART_MAIN);
    lv_obj_set_style_text_color(button, lv_color_hex(COLOR_ACCENT),
                                LV_PART_MAIN | LV_STATE_PRESSED);
    if (event_cb != NULL) {
        lv_obj_add_event_cb(button, event_cb, LV_EVENT_ALL,
                            (void *)(uintptr_t)right);
    }

    lv_obj_t *arrow = lv_label_create(button);
    lv_label_set_text(arrow, right ? ">" : "<");
    lv_obj_set_style_text_font(arrow, &lv_font_montserrat_48, LV_PART_MAIN);
    lv_obj_align(arrow, LV_ALIGN_CENTER, 0, -20);

    lv_obj_t *caption = lv_label_create(button);
    lv_label_set_text(caption, right ? "NEXT PAGE" : "PREVIOUS");
    lv_obj_set_style_text_font(caption, &lv_font_montserrat_14, LV_PART_MAIN);
    lv_obj_set_style_text_letter_space(caption, 1, LV_PART_MAIN);
    lv_obj_set_style_text_color(caption, lv_color_hex(COLOR_TEXT_MUTED),
                                LV_PART_MAIN);
    lv_obj_align(caption, LV_ALIGN_CENTER, 0, 48);
    return button;
}

void ui_screen_create(ui_screen_t *ui, lv_obj_t *screen,
                      lv_event_cb_t event_cb)
{
    lv_obj_set_style_bg_color(screen, lv_color_hex(COLOR_BACKGROUND),
                              LV_PART_MAIN);
    lv_obj_clear_flag(screen, LV_OBJ_FLAG_SCROLLABLE);

    make_button(screen, false, event_cb);
    make_button(screen, true, event_cb);

    lv_obj_t *title = lv_label_create(screen);
    lv_label_set_text(title, DEVICE_NAME);
    lv_obj_set_style_text_color(title, lv_color_hex(COLOR_TEXT), LV_PART_MAIN);
    lv_obj_align(title, LV_ALIGN_TOP_LEFT, 10, 15);

    lv_obj_t *divider = lv_obj_create(screen);
    lv_obj_remove_style_all(divider);
    lv_obj_set_size(divider, 300, 1);
    lv_obj_set_style_bg_color(divider, lv_color_hex(0x252d37), LV_PART_MAIN);
    lv_obj_set_style_bg_opa(divider, LV_OPA_COVER, LV_PART_MAIN);
    lv_obj_align(divider, LV_ALIGN_TOP_MID, 0, 43);
    lv_obj_clear_flag(divider, LV_OBJ_FLAG_CLICKABLE);

    lv_obj_t *status = lv_obj_create(screen);
    lv_obj_remove_style_all(status);
    lv_obj_set_size(status, 96, 26);
    lv_obj_set_style_radius(status, 13, LV_PART_MAIN);
    lv_obj_set_style_bg_color(status, lv_color_hex(COLOR_SURFACE), LV_PART_MAIN);
    lv_obj_set_style_bg_opa(status, LV_OPA_COVER, LV_PART_MAIN);
    lv_obj_set_style_border_width(status, 1, LV_PART_MAIN);
    lv_obj_set_style_border_color(status, lv_color_hex(COLOR_BORDER),
                                  LV_PART_MAIN);
    lv_obj_align(status, LV_ALIGN_TOP_RIGHT, -8, 8);
    lv_obj_clear_flag(status, LV_OBJ_FLAG_CLICKABLE);

    ui->status_dot = lv_obj_create(status);
    lv_obj_remove_style_all(ui->status_dot);
    lv_obj_set_size(ui->status_dot, 8, 8);
    lv_obj_set_style_radius(ui->status_dot, LV_RADIUS_CIRCLE, LV_PART_MAIN);
    style_status_dot(ui->status_dot, false);
    lv_obj_clear_flag(ui->status_dot, LV_OBJ_FLAG_CLICKABLE);
    lv_obj_align(ui->status_dot, LV_ALIGN_LEFT_MID, 9, 0);

    ui->status_label = lv_label_create(status);
    lv_label_set_text(ui->status_label, "PAIRING");
    lv_obj_set_style_text_font(ui->status_label, &lv_font_montserrat_14,
                               LV_PART_MAIN);
    lv_obj_set_style_text_color(ui->status_label,
                                lv_color_hex(COLOR_TEXT_MUTED), LV_PART_MAIN);
    lv_obj_align(ui->status_label, LV_ALIGN_LEFT_MID, 26, 0);

    ui->battery_label = lv_label_create(screen);
    lv_label_set_text(ui->battery_label, "");
    lv_obj_set_style_text_font(ui->battery_label, &lv_font_montserrat_14,
                               LV_PART_MAIN);
    lv_obj_set_style_text_color(ui->battery_label, lv_color_hex(0xa9824f),
                                LV_PART_MAIN);
    lv_obj_align(ui->battery_label, LV_ALIGN_TOP_RIGHT, -112, 14);
}

void ui_screen_set_connected(ui_screen_t *ui, bool connected)
{
    if (ui->status_dot != NULL) {
        style_status_dot(ui->status_dot, connected);
    }
    if (ui->status_label != NULL) {
        lv_label_set_text(ui->status_label, connected ? "READY" : "PAIRING");
        lv_obj_set_style_text_color(
            ui->status_label,
            lv_color_hex(connected ? COLOR_TEXT : COLOR_TEXT_MUTED),
            LV_PART_MAIN);
    }
}

void ui_screen_set_battery_text(ui_screen_t *ui, const char *text)
{
    if (ui->battery_label != NULL &&
            strcmp(lv_label_get_text(ui->battery_label), text) != 0) {
        lv_label_set_text(ui->battery_label, text);
    }
}

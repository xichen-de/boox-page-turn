#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "lvgl.h"
#include "ui_screen.h"

#define DISPLAY_WIDTH 320
#define DISPLAY_HEIGHT 240

static uint16_t s_framebuffer[DISPLAY_WIDTH * DISPLAY_HEIGHT];
static uint8_t s_draw_buffer[DISPLAY_WIDTH * DISPLAY_HEIGHT * sizeof(uint16_t)];

static void flush_cb(lv_display_t *display, const lv_area_t *area,
                     uint8_t *pixels)
{
    int32_t width = lv_area_get_width(area);
    const uint16_t *source = (const uint16_t *)pixels;

    for (int32_t y = area->y1; y <= area->y2; ++y) {
        uint16_t *destination = &s_framebuffer[y * DISPLAY_WIDTH + area->x1];
        memcpy(destination, source, (size_t)width * sizeof(*source));
        source += width;
    }
    lv_display_flush_ready(display);
}

static int write_ppm(const char *path)
{
    FILE *file = fopen(path, "wb");
    if (file == NULL) {
        perror(path);
        return 1;
    }

    fprintf(file, "P6\n%d %d\n255\n", DISPLAY_WIDTH, DISPLAY_HEIGHT);
    for (size_t index = 0; index < DISPLAY_WIDTH * DISPLAY_HEIGHT; ++index) {
        uint16_t value = s_framebuffer[index];
        uint8_t rgb[] = {
            (uint8_t)(((value >> 11) & 0x1fU) * 255U / 31U),
            (uint8_t)(((value >> 5) & 0x3fU) * 255U / 63U),
            (uint8_t)((value & 0x1fU) * 255U / 31U),
        };
        if (fwrite(rgb, sizeof(rgb), 1, file) != 1) {
            fclose(file);
            return 1;
        }
    }
    return fclose(file) == 0 ? 0 : 1;
}

int main(int argc, char **argv)
{
    if (argc != 2) {
        fprintf(stderr, "usage: %s OUTPUT.ppm\n", argv[0]);
        return 2;
    }

    lv_init();
    lv_display_t *display = lv_display_create(DISPLAY_WIDTH, DISPLAY_HEIGHT);
    if (display == NULL) {
        return 1;
    }
    lv_display_set_color_format(display, LV_COLOR_FORMAT_RGB565);
    lv_display_set_flush_cb(display, flush_cb);
    lv_display_set_buffers(display, s_draw_buffer, NULL, sizeof(s_draw_buffer),
                           LV_DISPLAY_RENDER_MODE_FULL);

    ui_screen_t ui = {0};
    ui_screen_create(&ui, lv_display_get_screen_active(display), NULL);
    ui_screen_set_connected(&ui, true);
    lv_obj_invalidate(lv_display_get_screen_active(display));
    lv_refr_now(display);

    int result = write_ppm(argv[1]);
    lv_display_delete(display);
    lv_deinit();
    return result;
}

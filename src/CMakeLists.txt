idf_component_register(
    SRCS "main.c" "ui.c" "ui_screen.c" "ble_hid.c" "power.c"
    INCLUDE_DIRS "."
    REQUIRES bt esp_hid nvs_flash m5stack_core_s3
)

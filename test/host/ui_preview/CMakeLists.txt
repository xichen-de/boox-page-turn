cmake_minimum_required(VERSION 3.16)
project(boox_remote_ui_preview LANGUAGES C)

enable_testing()

set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED ON)
set(CMAKE_C_EXTENSIONS OFF)

get_filename_component(PROJECT_ROOT "${CMAKE_CURRENT_LIST_DIR}/../../.." ABSOLUTE)
set(LVGL_ROOT "${PROJECT_ROOT}/managed_components/lvgl__lvgl")
if(NOT EXISTS "${LVGL_ROOT}/CMakeLists.txt")
    message(FATAL_ERROR "LVGL is not installed; run pio run first")
endif()

set(LV_BUILD_CONF_PATH "${CMAKE_CURRENT_LIST_DIR}/lv_conf.h" CACHE PATH "" FORCE)
set(CONFIG_LV_BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)
set(CONFIG_LV_BUILD_DEMOS OFF CACHE BOOL "" FORCE)
set(CONFIG_LV_USE_THORVG_INTERNAL OFF CACHE BOOL "" FORCE)
add_subdirectory("${LVGL_ROOT}" lvgl)

add_executable(render_boox_ui
    "${PROJECT_ROOT}/test/host/render_ui.c"
    "${PROJECT_ROOT}/src/ui_screen.c")

target_compile_options(render_boox_ui PRIVATE -Wall -Wextra -Werror -pedantic)
target_include_directories(render_boox_ui PRIVATE "${PROJECT_ROOT}/src")
target_link_libraries(render_boox_ui PRIVATE lvgl::lvgl)

add_executable(test_idle_policy "${PROJECT_ROOT}/test/host/test_idle_policy.c")
target_compile_options(test_idle_policy PRIVATE -Wall -Wextra -Werror -pedantic)
target_include_directories(test_idle_policy PRIVATE "${PROJECT_ROOT}/src")
add_test(NAME idle_policy COMMAND test_idle_policy)

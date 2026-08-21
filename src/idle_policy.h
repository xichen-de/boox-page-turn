#pragma once

#include <stdint.h>

#define IDLE_DIM_TIMEOUT_MS (5UL * 60UL * 1000UL)
#define IDLE_DISPLAY_OFF_TIMEOUT_MS (30UL * 60UL * 1000UL)
#define IDLE_POWER_OFF_TIMEOUT_MS (24UL * 60UL * 60UL * 1000UL)

typedef enum {
    IDLE_ACTION_ACTIVE,
    IDLE_ACTION_DIM_DISPLAY,
    IDLE_ACTION_TURN_DISPLAY_OFF,
    IDLE_ACTION_POWER_OFF,
} idle_action_t;

static inline uint32_t idle_elapsed_ms(uint32_t now_ms,
                                       uint32_t last_interaction_ms)
{
    /* Unsigned subtraction also handles one rollover of the millisecond clock. */
    return now_ms - last_interaction_ms;
}

static inline idle_action_t idle_action_for_elapsed(uint32_t elapsed_ms)
{
    if (elapsed_ms >= IDLE_POWER_OFF_TIMEOUT_MS) {
        return IDLE_ACTION_POWER_OFF;
    }
    if (elapsed_ms >= IDLE_DISPLAY_OFF_TIMEOUT_MS) {
        return IDLE_ACTION_TURN_DISPLAY_OFF;
    }
    if (elapsed_ms >= IDLE_DIM_TIMEOUT_MS) {
        return IDLE_ACTION_DIM_DISPLAY;
    }
    return IDLE_ACTION_ACTIVE;
}

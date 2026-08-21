#include <stdint.h>
#include <stdio.h>

#include "idle_policy.h"

static int s_failures;

static void expect_action(const char *name, uint32_t elapsed_ms,
                          idle_action_t expected)
{
    idle_action_t actual = idle_action_for_elapsed(elapsed_ms);
    if (actual != expected) {
        fprintf(stderr, "%s: expected action %d, got %d\n", name,
                (int)expected, (int)actual);
        ++s_failures;
    }
}

static void test_thresholds(void)
{
    expect_action("initially active", 0, IDLE_ACTION_ACTIVE);
    expect_action("before dim", IDLE_DIM_TIMEOUT_MS - 1,
                  IDLE_ACTION_ACTIVE);
    expect_action("at dim", IDLE_DIM_TIMEOUT_MS, IDLE_ACTION_DIM_DISPLAY);
    expect_action("before display off", IDLE_DISPLAY_OFF_TIMEOUT_MS - 1,
                  IDLE_ACTION_DIM_DISPLAY);
    expect_action("at display off", IDLE_DISPLAY_OFF_TIMEOUT_MS,
                  IDLE_ACTION_TURN_DISPLAY_OFF);
    expect_action("before power off", IDLE_POWER_OFF_TIMEOUT_MS - 1,
                  IDLE_ACTION_TURN_DISPLAY_OFF);
    expect_action("at power off", IDLE_POWER_OFF_TIMEOUT_MS,
                  IDLE_ACTION_POWER_OFF);
}

static void test_clock_rollover(void)
{
    uint32_t last_interaction_ms = UINT32_MAX - 99U;
    uint32_t now_ms = 100U;
    uint32_t elapsed_ms = idle_elapsed_ms(now_ms, last_interaction_ms);

    if (elapsed_ms != 200U) {
        fprintf(stderr, "clock rollover: expected 200 ms, got %lu ms\n",
                (unsigned long)elapsed_ms);
        ++s_failures;
    }
    expect_action("active after rollover", elapsed_ms, IDLE_ACTION_ACTIVE);
}

int main(void)
{
    test_thresholds();
    test_clock_rollover();

    if (s_failures != 0) {
        fprintf(stderr, "%d idle policy test(s) failed\n", s_failures);
        return 1;
    }
    puts("idle policy tests passed");
    return 0;
}

/**
 * @file test_ring_buffer.c
 * @brief Unit tests for the lock-free ring buffer.
 *
 * EXAMPLE FILE — demonstrates the Unity test pattern for this project.
 * Copy this as a starting point for each new module's test file.
 *
 * TDD workflow:
 *   1. Write a test below (RED)
 *   2. Build & run — watch it fail
 *   3. Write minimal code in src/core/ring_buffer.c (GREEN)
 *   4. Refactor, re-run, stay green
 */

#include "unity.h"

/* Include the module under test */
/* #include "core/ring_buffer.h" */

/* ── Test cases ───────────────────────────────────────────────────────── */

/**
 * Template test — replace with real tests.
 *
 * Naming convention: test_<module>_<behavior_under_test>
 * Examples:
 *   test_ring_buffer_write_advances_head
 *   test_ring_buffer_read_returns_oldest_sample
 *   test_ring_buffer_full_overwrites_tail
 *   test_ring_buffer_empty_returns_zero
 */
void test_ring_buffer_placeholder(void) {
    TEST_IGNORE_MESSAGE("Ring buffer not yet implemented — start here");
}

/* ── Test group runner (called from test_main.c) ──────────────────────── */

void run_test_ring_buffer(void) {
    RUN_TEST(test_ring_buffer_placeholder);

    /* Add tests as you write them: */
    /* RUN_TEST(test_ring_buffer_write_advances_head); */
    /* RUN_TEST(test_ring_buffer_read_returns_oldest_sample); */
    /* RUN_TEST(test_ring_buffer_full_overwrites_tail); */
}

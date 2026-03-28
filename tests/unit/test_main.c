/**
 * @file test_main.c
 * @brief Unity test runner — calls all test groups.
 *
 * Add new test group runners here as you create them.
 * Each test file exposes a void run_test_<module>(void) function.
 */

#include "unity.h"

/* ── Forward-declare test group runners ───────────────────────────────── */
/* Uncomment as you add test files: */
/* extern void run_test_ring_buffer(void); */
/* extern void run_test_fft(void); */
/* extern void run_test_mfcc(void); */

/* ── Unity setup/teardown (called per test) ───────────────────────────── */

void setUp(void) {
    /* Called before each TEST — reset shared state here if needed */
}

void tearDown(void) {
    /* Called after each TEST — cleanup here if needed */
}

/* ── Placeholder test so the runner compiles immediately ──────────────── */

void test_placeholder_should_pass(void) {
    TEST_ASSERT_TRUE_MESSAGE(1, "Template placeholder — replace me with real tests");
}

/* ── Main ─────────────────────────────────────────────────────────────── */

int main(void) {
    UNITY_BEGIN();

    RUN_TEST(test_placeholder_should_pass);

    /* Uncomment as you add test groups: */
    /* run_test_ring_buffer(); */
    /* run_test_fft(); */

    return UNITY_END();
}

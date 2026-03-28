/**
 * @file hal_qemu.c
 * @brief QEMU Cortex-M4 HAL implementation using semihosting.
 *
 * Semihosting lets code running in QEMU make host OS calls (printf, file I/O)
 * through a debug channel. This gives us printf-style debugging and file-based
 * audio I/O without real hardware.
 *
 * Build with: arm-none-eabi-gcc -specs=rdimon.specs -lrdimon
 * Run with:   qemu-system-arm -machine lm3s6965evb -semihosting -nographic -kernel ...
 */

#include "platform/hal.h"

#include <stdarg.h>
#include <stdio.h>
#include <string.h>

/* ── ARM DWT cycle counter for timing ─────────────────────────────────── */

/* Cortex-M4 Data Watchpoint and Trace unit registers */
#define DWT_CTRL   (*(volatile uint32_t*)0xE0001000)
#define DWT_CYCCNT (*(volatile uint32_t*)0xE0001004)
#define SCB_DEMCR  (*(volatile uint32_t*)0xE000EDFC)

/* Assumed CPU clock in QEMU — LM3S6965 default is 50MHz */
#define CPU_FREQ_HZ 50000000UL

static char s_last_error[256] = {0};
static uint32_t s_cyccnt_offset = 0;

static void set_error(const char* fmt, ...) {
    va_list args;
    va_start(args, fmt);
    vsnprintf(s_last_error, sizeof(s_last_error), fmt, args);
    va_end(args);
}

/* ── Platform lifecycle ───────────────────────────────────────────────── */

hal_status_t hal_init(void) {
    /* Enable DWT cycle counter */
    SCB_DEMCR |= (1 << 24);    /* TRCENA */
    DWT_CTRL  |= 1;             /* CYCCNTENA */
    s_cyccnt_offset = DWT_CYCCNT;

    return HAL_OK;
}

void hal_deinit(void) {
    /* nothing to tear down */
}

const char* hal_get_last_error(void) {
    return s_last_error;
}

/* ── Timing ───────────────────────────────────────────────────────────── */

uint64_t hal_micros(void) {
    uint32_t cycles = DWT_CYCCNT - s_cyccnt_offset;
    return (uint64_t)cycles / (CPU_FREQ_HZ / 1000000UL);
}

void hal_delay_us(uint32_t us) {
    uint64_t start = hal_micros();
    while ((hal_micros() - start) < us) {
        /* spin */
    }
}

/* ── Audio I/O (file-based for QEMU) ──────────────────────────────────── */

/**
 * In QEMU mode, audio is driven by files, not real-time hardware.
 * hal_audio_start() reads a WAV from the semihosted filesystem
 * and feeds it through the callback in buffer-sized chunks.
 * This lets integration tests run the full pipeline in CI.
 */

hal_status_t hal_audio_start(
    const hal_audio_config_t* config,
    hal_audio_callback_t callback
) {
    if (!config || !callback) {
        set_error("NULL config or callback");
        return HAL_ERR_INVALID_PARAM;
    }

    /*
     * TODO: implement file-based audio streaming for QEMU:
     *   1. Open "test_input.raw" via semihosting fopen
     *   2. Read buffer_frames * channels * sizeof(int16_t) bytes per iteration
     *   3. Call callback(input_buf, output_buf, buffer_frames)
     *   4. Write output_buf to "test_output.raw"
     *   5. Loop until EOF
     *
     * For now, return ERR so tests know this is a stub.
     */
    set_error("QEMU audio: not yet implemented — see TODO in hal_qemu.c");
    return HAL_ERR_PLATFORM;
}

hal_status_t hal_audio_stop(void) {
    return HAL_OK;
}

/* ── Debug (semihosting printf) ───────────────────────────────────────── */

void hal_debug_printf(const char* fmt, ...) {
    va_list args;
    va_start(args, fmt);
    vfprintf(stderr, fmt, args);  /* semihosting redirects stderr to host */
    va_end(args);
}

/* ── GPIO (simulated, writes to debug output) ─────────────────────────── */

static uint8_t s_gpio_state[64] = {0};

hal_status_t hal_gpio_init(uint16_t pin, hal_gpio_mode_t mode) {
    if (pin >= 64) return HAL_ERR_INVALID_PARAM;
    hal_debug_printf("[QEMU GPIO] pin %u mode %s\n",
        pin, mode == HAL_GPIO_MODE_OUTPUT ? "OUTPUT" : "INPUT");
    return HAL_OK;
}

void hal_gpio_write(uint16_t pin, uint8_t value) {
    if (pin < 64) s_gpio_state[pin] = value;
}

uint8_t hal_gpio_read(uint16_t pin) {
    if (pin >= 64) return 0;
    return s_gpio_state[pin];
}

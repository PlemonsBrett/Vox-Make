/**
 * @file hal.h
 * @brief Hardware Abstraction Layer — platform-independent interface.
 *
 * Every platform backend (native, qemu, stm32f4) implements these functions.
 * Core DSP code NEVER calls OS/hardware directly — only through this interface.
 *
 * Design rules:
 *   - All functions are C-linkage (callable from C17 core code)
 *   - No heap allocation in any HAL implementation's hot path
 *   - Timing functions return microseconds as uint64_t
 *   - Audio callbacks must complete within one buffer period
 */

#ifndef PLATFORM_HAL_H
#define PLATFORM_HAL_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stddef.h>
#include <stdint.h>

/* ── Status codes ─────────────────────────────────────────────────────── */

typedef enum {
    HAL_OK = 0,
    HAL_ERR_NOT_INITIALIZED,
    HAL_ERR_INVALID_PARAM,
    HAL_ERR_DEVICE_NOT_FOUND,
    HAL_ERR_BUFFER_OVERRUN,
    HAL_ERR_TIMEOUT,
    HAL_ERR_PLATFORM,       /* platform-specific error, check hal_get_last_error() */
} hal_status_t;

/* ── Platform lifecycle ───────────────────────────────────────────────── */

/**
 * Initialize platform hardware. Call once at startup.
 * @return HAL_OK on success.
 */
hal_status_t hal_init(void);

/**
 * Tear down platform hardware. Call once at shutdown.
 */
void hal_deinit(void);

/**
 * Get human-readable description of last platform-specific error.
 * @return Pointer to static string (do not free).
 */
const char* hal_get_last_error(void);

/* ── Timing ───────────────────────────────────────────────────────────── */

/**
 * Microseconds since hal_init(). Wraps at ~584,942 years on 64-bit.
 */
uint64_t hal_micros(void);

/**
 * Busy-wait for the specified number of microseconds.
 * On native: nanosleep. On embedded: cycle-counting.
 */
void hal_delay_us(uint32_t us);

/* ── Audio I/O ────────────────────────────────────────────────────────── */

/**
 * Audio configuration for hal_audio_start().
 */
typedef struct {
    uint32_t sample_rate;    /* Hz: 8000, 16000, 22050, 44100, 48000       */
    uint16_t channels;       /* 1 = mono, 2 = stereo                       */
    uint16_t bit_depth;      /* 16 or 32                                   */
    uint16_t buffer_frames;  /* frames per callback (e.g., 256, 512, 1024) */
} hal_audio_config_t;

/**
 * Audio callback signature.
 * Called from audio thread/ISR — must be lock-free, no heap, no printf.
 *
 * @param input   Interleaved input samples (NULL if capture not supported)
 * @param output  Interleaved output buffer to fill (NULL if playback not supported)
 * @param frames  Number of frames (samples per channel) in this callback
 */
typedef void (*hal_audio_callback_t)(
    const int16_t* input,
    int16_t* output,
    uint32_t frames
);

/**
 * Open audio device and begin streaming.
 * Callback fires from a dedicated audio thread or ISR.
 */
hal_status_t hal_audio_start(
    const hal_audio_config_t* config,
    hal_audio_callback_t callback
);

/**
 * Stop audio streaming and close device.
 */
hal_status_t hal_audio_stop(void);

/* ── Debug / Logging ──────────────────────────────────────────────────── */

/**
 * Platform-appropriate debug output.
 * Native: fprintf(stderr, ...). QEMU: semihosting. STM32: SWO/UART.
 * NOT safe to call from audio callback.
 */
void hal_debug_printf(const char* fmt, ...);

/* ── GPIO (embedded targets only, no-op on native/qemu) ───────────────── */

typedef enum {
    HAL_GPIO_MODE_INPUT,
    HAL_GPIO_MODE_OUTPUT,
} hal_gpio_mode_t;

hal_status_t hal_gpio_init(uint16_t pin, hal_gpio_mode_t mode);
void hal_gpio_write(uint16_t pin, uint8_t value);
uint8_t hal_gpio_read(uint16_t pin);

#ifdef __cplusplus
}
#endif

#endif /* PLATFORM_HAL_H */

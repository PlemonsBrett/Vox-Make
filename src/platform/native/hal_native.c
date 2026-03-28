/**
 * @file hal_native.c
 * @brief Native (desktop) HAL implementation.
 *
 * Uses PortAudio for audio I/O, clock_gettime for timing.
 * This is the target you use for day-to-day TDD and development.
 */

/* clock_gettime, CLOCK_MONOTONIC, nanosleep — visible only with POSIX feature test. */
#if !defined(_WIN32)
#define _POSIX_C_SOURCE 200809L
#endif

#include "platform/hal.h"

#include <stdarg.h>
#include <stdio.h>
#include <string.h>
#include <time.h>

/* ── If PortAudio is available, include it ────────────────────────────── */
#ifdef HAS_PORTAUDIO
#include <portaudio.h>

static PaStream* s_stream = NULL;
static hal_audio_callback_t s_user_callback = NULL;

static int pa_callback(
    const void* input,
    void* output,
    unsigned long frame_count,
    const PaStreamCallbackTimeInfo* time_info,
    PaStreamCallbackFlags flags,
    void* user_data
) {
    (void)time_info;
    (void)flags;
    (void)user_data;

    s_user_callback(
        (const int16_t*)input,
        (int16_t*)output,
        (uint32_t)frame_count
    );
    return paContinue;
}
#endif /* HAS_PORTAUDIO */

/* ── Error tracking ───────────────────────────────────────────────────── */

static char s_last_error[256] = {0};

static void set_error(const char* fmt, ...) {
    va_list args;
    va_start(args, fmt);
    vsnprintf(s_last_error, sizeof(s_last_error), fmt, args);
    va_end(args);
}

/* ── Timing state ─────────────────────────────────────────────────────── */

static struct timespec s_start_time;

/* ── Platform lifecycle ───────────────────────────────────────────────── */

hal_status_t hal_init(void) {
    clock_gettime(CLOCK_MONOTONIC, &s_start_time);

#ifdef HAS_PORTAUDIO
    PaError err = Pa_Initialize();
    if (err != paNoError) {
        set_error("PortAudio init failed: %s", Pa_GetErrorText(err));
        return HAL_ERR_PLATFORM;
    }
#endif

    return HAL_OK;
}

void hal_deinit(void) {
#ifdef HAS_PORTAUDIO
    if (s_stream) {
        hal_audio_stop();
    }
    Pa_Terminate();
#endif
}

const char* hal_get_last_error(void) {
    return s_last_error;
}

/* ── Timing ───────────────────────────────────────────────────────────── */

uint64_t hal_micros(void) {
    struct timespec now;
    clock_gettime(CLOCK_MONOTONIC, &now);

    uint64_t sec  = (uint64_t)(now.tv_sec - s_start_time.tv_sec);
    int64_t nsec  = now.tv_nsec - s_start_time.tv_nsec;

    if (nsec < 0) {
        sec  -= 1;
        nsec += 1000000000L;
    }

    return sec * 1000000ULL + (uint64_t)(nsec / 1000);
}

void hal_delay_us(uint32_t us) {
    struct timespec ts;
    ts.tv_sec  = us / 1000000;
    ts.tv_nsec = (us % 1000000) * 1000L;
    nanosleep(&ts, NULL);
}

/* ── Audio I/O ────────────────────────────────────────────────────────── */

hal_status_t hal_audio_start(
    const hal_audio_config_t* config,
    hal_audio_callback_t callback
) {
    if (!config || !callback) {
        set_error("NULL config or callback");
        return HAL_ERR_INVALID_PARAM;
    }

#ifdef HAS_PORTAUDIO
    s_user_callback = callback;

    PaError err = Pa_OpenDefaultStream(
        &s_stream,
        (int)config->channels,   /* input channels  */
        (int)config->channels,   /* output channels */
        paInt16,
        (double)config->sample_rate,
        config->buffer_frames,
        pa_callback,
        NULL
    );
    if (err != paNoError) {
        set_error("Pa_OpenDefaultStream: %s", Pa_GetErrorText(err));
        return HAL_ERR_DEVICE_NOT_FOUND;
    }

    err = Pa_StartStream(s_stream);
    if (err != paNoError) {
        set_error("Pa_StartStream: %s", Pa_GetErrorText(err));
        Pa_CloseStream(s_stream);
        s_stream = NULL;
        return HAL_ERR_PLATFORM;
    }

    return HAL_OK;
#else
    set_error("Built without PortAudio support");
    return HAL_ERR_PLATFORM;
#endif
}

hal_status_t hal_audio_stop(void) {
#ifdef HAS_PORTAUDIO
    if (s_stream) {
        Pa_StopStream(s_stream);
        Pa_CloseStream(s_stream);
        s_stream = NULL;
        s_user_callback = NULL;
    }
    return HAL_OK;
#else
    return HAL_ERR_NOT_INITIALIZED;
#endif
}

/* ── Debug ────────────────────────────────────────────────────────────── */

void hal_debug_printf(const char* fmt, ...) {
    va_list args;
    va_start(args, fmt);
    vfprintf(stderr, fmt, args);
    va_end(args);
}

/* ── GPIO (no-op on desktop) ──────────────────────────────────────────── */

hal_status_t hal_gpio_init(uint16_t pin, hal_gpio_mode_t mode) {
    (void)pin;
    (void)mode;
    return HAL_OK;
}

void hal_gpio_write(uint16_t pin, uint8_t value) {
    (void)pin;
    (void)value;
}

uint8_t hal_gpio_read(uint16_t pin) {
    (void)pin;
    return 0;
}

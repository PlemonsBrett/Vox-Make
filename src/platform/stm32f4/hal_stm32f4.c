/**
 * @file hal_stm32f4.c
 * @brief STM32F4 HAL implementation — real hardware target.
 *
 * This wraps STM32 HAL/LL drivers. When you design a board around
 * your STM32 chip (or use a BeagleBone with an audio cape), the
 * pin mappings and peripheral config go here.
 *
 * Build with: arm-none-eabi-gcc + STM32CubeF4 HAL
 *
 * Stubbed for now — fill in when hardware is ready.
 */

#include "platform/hal.h"

#include <stdarg.h>
#include <stdio.h>

/*
 * TODO: Include STM32 HAL headers when hardware is ready:
 *   #include "stm32f4xx_hal.h"
 *   #include "stm32f4xx_ll_dma.h"
 */

/* ── DWT cycle counter (same as QEMU, real hardware) ──────────────────── */

#define DWT_CTRL   (*(volatile uint32_t*)0xE0001000)
#define DWT_CYCCNT (*(volatile uint32_t*)0xE0001004)
#define SCB_DEMCR  (*(volatile uint32_t*)0xE000EDFC)

/* STM32F4 typical clock — adjust to your board's actual frequency */
#ifndef CPU_FREQ_HZ
#define CPU_FREQ_HZ 168000000UL
#endif

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
    /*
     * TODO: real hardware init sequence:
     *   HAL_Init();
     *   SystemClock_Config();  // your CubeMX-generated clock setup
     *   MX_GPIO_Init();
     *   MX_DMA_Init();
     *   MX_I2S_Init();         // or SAI for audio
     */

    /* Enable DWT cycle counter */
    SCB_DEMCR |= (1 << 24);
    DWT_CTRL  |= 1;
    s_cyccnt_offset = DWT_CYCCNT;

    return HAL_OK;
}

void hal_deinit(void) {
    /* TODO: HAL_DeInit(); */
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
        __asm volatile("nop");
    }
}

/* ── Audio I/O ────────────────────────────────────────────────────────── */

/*
 * TODO: On real STM32F4 hardware, audio typically uses:
 *   - I2S or SAI peripheral for the audio codec (e.g., WM8731, CS4344)
 *   - DMA double-buffering (HAL_I2S_Transmit_DMA / HAL_I2S_Receive_DMA)
 *   - HAL_I2S_TxHalfCpltCallback / HAL_I2S_TxCpltCallback for ping-pong
 *
 * The callback from hal.h maps directly to the DMA half/complete ISR.
 */

hal_status_t hal_audio_start(
    const hal_audio_config_t* config,
    hal_audio_callback_t callback
) {
    if (!config || !callback) {
        set_error("NULL config or callback");
        return HAL_ERR_INVALID_PARAM;
    }

    set_error("STM32F4 audio: not yet implemented — need board bring-up");
    return HAL_ERR_PLATFORM;
}

hal_status_t hal_audio_stop(void) {
    return HAL_OK;
}

/* ── Debug (SWO / UART) ──────────────────────────────────────────────── */

/*
 * TODO: Route to SWO (ITM_SendChar) or UART depending on board.
 * For now, this is a no-op that compiles cleanly.
 */

void hal_debug_printf(const char* fmt, ...) {
    (void)fmt;
    /* TODO: implement SWO or UART printf */
}

/* ── GPIO ─────────────────────────────────────────────────────────────── */

hal_status_t hal_gpio_init(uint16_t pin, hal_gpio_mode_t mode) {
    (void)pin;
    (void)mode;
    /* TODO: HAL_GPIO_Init with pin mapping table */
    return HAL_OK;
}

void hal_gpio_write(uint16_t pin, uint8_t value) {
    (void)pin;
    (void)value;
    /* TODO: HAL_GPIO_WritePin */
}

uint8_t hal_gpio_read(uint16_t pin) {
    (void)pin;
    /* TODO: HAL_GPIO_ReadPin */
    return 0;
}

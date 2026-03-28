/**
 * @file main.cpp
 * @brief Application entry point (C++17).
 *
 * This is the top-level orchestration layer. It:
 *   1. Initializes the platform via HAL
 *   2. Configures the processing pipeline
 *   3. Runs the main loop (or starts audio streaming)
 *   4. Cleans up on exit
 *
 * All DSP work happens in the C17 core library.
 * This file handles CLI parsing, RAII wrappers, and lifecycle.
 */

extern "C" {
#include "platform/hal.h"
}

#include <cstdio>
#include <cstdlib>

namespace {

/** RAII wrapper for HAL lifecycle */
class Platform {
public:
    Platform() {
        hal_status_t status = hal_init();
        if (status != HAL_OK) {
            std::fprintf(stderr, "HAL init failed: %s\n", hal_get_last_error());
            std::exit(1);
        }
    }

    ~Platform() {
        hal_deinit();
    }

    /* Non-copyable, non-movable */
    Platform(const Platform&) = delete;
    Platform& operator=(const Platform&) = delete;
    Platform(Platform&&) = delete;
    Platform& operator=(Platform&&) = delete;
};

} /* anonymous namespace */

int main(int argc, char* argv[]) {
    (void)argc;
    (void)argv;

    Platform platform;

    hal_debug_printf("vox-make v0.1.0 initialized\n");
    hal_debug_printf("Platform time: %llu us\n",
        (unsigned long long)hal_micros());

    /*
     * TODO: Replace with your pipeline:
     *   1. Parse CLI args (--input file.wav, --model template.bin, etc.)
     *   2. Configure audio (hal_audio_start with your callback)
     *   3. Run main loop or wait for audio callbacks
     *   4. Report results
     */

    hal_debug_printf("No pipeline configured — exiting.\n");
    return 0;
}

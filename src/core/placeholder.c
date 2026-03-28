/**
 * @file placeholder.c
 * @brief Holds a non-empty translation unit so the `core` static library
 *        configures and builds until real DSP sources are added.
 *
 * Remove this file from CMake once at least one real C source exists under src/core/.
 */
#if defined(__GNUC__) || defined(__clang__)
__attribute__((unused))
#endif
static void vox_core_placeholder(void) {}

cmake_minimum_required(VERSION 3.22)

# ── Project definition ────────────────────────────────────────────────────
project(vox-make
    VERSION 0.1.0
    LANGUAGES C CXX
)

set(CMAKE_C_STANDARD 17)
set(CMAKE_C_STANDARD_REQUIRED ON)
set(CMAKE_C_EXTENSIONS OFF)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# ── Build target selection ────────────────────────────────────────────────
# Set via preset or -DTARGET_PLATFORM=native|qemu|stm32f4
set(TARGET_PLATFORM "native" CACHE STRING "Build target: native, qemu, stm32f4")
set_property(CACHE TARGET_PLATFORM PROPERTY STRINGS native qemu stm32f4)

message(STATUS "Target platform: ${TARGET_PLATFORM}")

# ── Compiler warnings (strict for all targets) ───────────────────────────
add_compile_options(
    -Wall -Wextra -Wpedantic -Werror
    -Wconversion -Wshadow -Wdouble-promotion
    -Wformat=2 -Wformat-truncation
    -fno-common
)

# Embedded-specific: no exceptions, no RTTI, minimize binary size
# -fno-exceptions / -fno-rtti are C++-only; applying them to C sources trips -Werror on GCC.
if(NOT TARGET_PLATFORM STREQUAL "native")
    add_compile_options(
        "$<$<COMPILE_LANGUAGE:CXX>:-fno-exceptions>"
        "$<$<COMPILE_LANGUAGE:CXX>:-fno-rtti>"
        -fno-unwind-tables
        -ffunction-sections -fdata-sections
        -Os
    )
    add_link_options(
        -Wl,--gc-sections
        -Wl,-Map=${CMAKE_BINARY_DIR}/${PROJECT_NAME}.map
    )
endif()

# ── Third-party: Unity test framework ────────────────────────────────────
add_subdirectory(third_party/unity)

# ── Core DSP library (C17, platform-independent) ─────────────────────────
add_library(core STATIC
    src/core/placeholder.c
    # src/core/ring_buffer.c
    # src/core/fft.c
    # src/core/mfcc.c
)

target_include_directories(core
    PUBLIC
        src/core/include
        src/platform/include     # for hal.h — core CAN call HAL
)

# Disable C++ features in core — enforce pure C17
set_target_properties(core PROPERTIES
    LINKER_LANGUAGE C
)

# ── Platform HAL library ─────────────────────────────────────────────────
if(TARGET_PLATFORM STREQUAL "native")
    add_library(hal STATIC src/platform/native/hal_native.c)

    # Try to find PortAudio — optional
    find_package(PkgConfig QUIET)
    if(PkgConfig_FOUND)
        pkg_check_modules(PORTAUDIO portaudio-2.0)
    endif()

    if(PORTAUDIO_FOUND)
        target_compile_definitions(hal PRIVATE HAS_PORTAUDIO)
        target_link_libraries(hal PRIVATE ${PORTAUDIO_LIBRARIES})
        target_include_directories(hal PRIVATE ${PORTAUDIO_INCLUDE_DIRS})
        message(STATUS "PortAudio found — audio I/O enabled")
    else()
        message(STATUS "PortAudio not found — audio I/O disabled (file-only mode)")
    endif()

elseif(TARGET_PLATFORM STREQUAL "qemu")
    add_library(hal STATIC src/platform/qemu/hal_qemu.c)
    # Semihosting link flags set in toolchain file

elseif(TARGET_PLATFORM STREQUAL "stm32f4")
    add_library(hal STATIC src/platform/stm32f4/hal_stm32f4.c)
    # STM32 HAL/CMSIS paths set in toolchain file
endif()

target_include_directories(hal
    PUBLIC src/platform/include
)

# ── Application (C++17) ──────────────────────────────────────────────────
add_executable(${PROJECT_NAME}
    src/app/main.cpp
)

target_include_directories(${PROJECT_NAME}
    PRIVATE src/app/include
)

target_link_libraries(${PROJECT_NAME}
    PRIVATE core hal
)

# ── Tests (native only) ──────────────────────────────────────────────────
if(TARGET_PLATFORM STREQUAL "native")
    enable_testing()

    add_executable(test_runner
        tests/unit/test_main.c
        # Add test files here:
        # tests/unit/test_ring_buffer.c
        # tests/unit/test_fft.c
    )

    target_link_libraries(test_runner
        PRIVATE core hal unity
    )

    target_include_directories(test_runner
        PRIVATE
            src/core/include
            src/platform/include
    )

    add_test(NAME unit_tests COMMAND test_runner)
endif()

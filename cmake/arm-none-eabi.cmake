# cmake/arm-none-eabi.cmake
# ARM Cortex-M4 cross-compilation toolchain
#
# Install: sudo apt install gcc-arm-none-eabi
# Or:      brew install arm-none-eabi-gcc (macOS)

set(CMAKE_SYSTEM_NAME Generic)
set(CMAKE_SYSTEM_PROCESSOR arm)

# ── Toolchain binaries ────────────────────────────────────────────────────
set(CMAKE_C_COMPILER   arm-none-eabi-gcc)
set(CMAKE_CXX_COMPILER arm-none-eabi-g++)
set(CMAKE_ASM_COMPILER arm-none-eabi-gcc)
set(CMAKE_OBJCOPY      arm-none-eabi-objcopy)
set(CMAKE_OBJDUMP      arm-none-eabi-objdump)
set(CMAKE_SIZE         arm-none-eabi-size)

# ── CPU flags for Cortex-M4 with hardware FPU ────────────────────────────
set(CPU_FLAGS "-mcpu=cortex-m4 -mthumb -mfloat-abi=hard -mfpu=fpv4-sp-d16")
set(CMAKE_C_FLAGS_INIT   "${CPU_FLAGS}")
set(CMAKE_CXX_FLAGS_INIT "${CPU_FLAGS}")
set(CMAKE_ASM_FLAGS_INIT "${CPU_FLAGS}")

# ── Linker ────────────────────────────────────────────────────────────────
if(TARGET_PLATFORM STREQUAL "qemu")
    # Semihosting: use rdimon specs for printf/file I/O via QEMU host
    set(CMAKE_EXE_LINKER_FLAGS_INIT
        "${CPU_FLAGS} --specs=rdimon.specs -lrdimon -lc -lm -lnosys"
    )
else()
    # Bare-metal: use nosys specs (no OS)
    set(CMAKE_EXE_LINKER_FLAGS_INIT
        "${CPU_FLAGS} --specs=nosys.specs -lc -lm -lnosys"
    )
    # TODO: Add your linker script: -T${CMAKE_SOURCE_DIR}/cmake/stm32f4.ld
endif()

# ── Prevent CMake from trying to link a test executable during config ─────
set(CMAKE_TRY_COMPILE_TARGET_TYPE STATIC_LIBRARY)

# ── Search paths: don't look for host programs, do look for ARM libs ──────
set(CMAKE_FIND_ROOT_PATH_MODE_PROGRAM NEVER)
set(CMAKE_FIND_ROOT_PATH_MODE_LIBRARY ONLY)
set(CMAKE_FIND_ROOT_PATH_MODE_INCLUDE ONLY)
set(CMAKE_FIND_ROOT_PATH_MODE_PACKAGE ONLY)

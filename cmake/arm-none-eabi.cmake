# cmake/arm-none-eabi.cmake
# ARM Cortex-M4 cross-compilation toolchain
#
# Install (Debian/Ubuntu): sudo apt install gcc-arm-none-eabi
# Install (Fedora):        sudo dnf install arm-none-eabi-gcc-cs-c++   # C + C++ (main app is .cpp)
# Install (macOS):         brew install arm-none-eabi-gcc
#
# The application links C++ (src/app/main.cpp). You need the C++ cross-compiler (provides cc1plus),
# not only arm-none-eabi-gcc.

set(CMAKE_SYSTEM_NAME Generic)
set(CMAKE_SYSTEM_PROCESSOR arm)

# ── Toolchain binaries ────────────────────────────────────────────────────
# Resolve via PATH so ccache wrappers (e.g. /usr/lib64/ccache/arm-none-eabi-gcc) work.
find_program(CMAKE_C_COMPILER arm-none-eabi-gcc REQUIRED)
find_program(_ARM_GXX arm-none-eabi-g++)
if(NOT _ARM_GXX)
    message(FATAL_ERROR
        "arm-none-eabi-g++ not found (required for C++ sources). "
        "The C-only package is not enough — you need the C++ frontend (cc1plus).\n"
        "  Fedora:    sudo dnf install arm-none-eabi-gcc-cs-c++\n"
        "  Debian/Ubuntu: sudo apt install gcc-arm-none-eabi\n"
        "  macOS:     brew install arm-none-eabi-gcc")
endif()
set(CMAKE_CXX_COMPILER "${_ARM_GXX}" CACHE FILEPATH "ARM none eabi C++ compiler" FORCE)
set(CMAKE_ASM_COMPILER "${CMAKE_C_COMPILER}" CACHE FILEPATH "ARM none eabi assembler" FORCE)
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

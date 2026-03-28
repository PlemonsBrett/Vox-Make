# vox-make

> Lightweight wake-word detection engine} project}

## Architecture

```plaintext
┌─────────────────────────────────────────────────────┐
│                  Application (C++17)                │
│         CLI / main loop / pipeline orchestration    │
├─────────────────────────────────────────────────────┤
│                   Core DSP (C17)                    │
│    Algorithms, math, signal processing, models      │
├──────────┬──────────────┬───────────────────────────┤
│  Native  │    QEMU      │     STM32 / BeagleBone    │
│ (desktop)│  (emulated)  │       (real hardware)     │
│PortAudio │  semihosting │    HAL / BSP drivers      │
└──────────┴──────────────┴───────────────────────────┘
```

**Core DSP** is pure C17 — no heap, no OS calls, no platform deps.  
**Application** is C++17 — RAII wrappers, CLI, orchestration.  
**Platform backends** implement `platform/hal.h` for each target.

## Build Targets

| Target     | What                           | Use When                        |
|------------|--------------------------------|---------------------------------|
| `native`   | Desktop build, links PortAudio | Day-to-day development & TDD    |
| `qemu`     | ARM Cortex-M4 on QEMU          | CI, pre-hardware validation     |
| `stm32f4`  | Real STM32F4 hardware          | Final integration & profiling   |

## Quick Start

```bash
# --- CMake (option A) ---
cmake --preset native && cmake --build --preset native
ctest --preset native

# --- Meson (option B) ---
meson setup builddir --native-file meson/native.ini
meson compile -C builddir
meson test -C builddir

# --- Python tools ---
cd tools && uv sync && uv run pytest
```

## Project Layout

```plaintext
├── src/
│   ├── core/              # C17 — platform-independent DSP/algorithms
│   │   ├── include/core/  # Public C headers
│   │   └── *.c
│   ├── app/               # C++17 — application layer
│   │   ├── include/app/   # Public C++ headers
│   │   └── *.cpp
│   └── platform/          # HAL implementations per target
│       ├── include/platform/
│       │   └── hal.h      # Abstract hardware interface
│       ├── native/        # Desktop (PortAudio, file I/O)
│       ├── qemu/          # QEMU semihosting
│       └── stm32f4/       # STM32 HAL/BSP
├── tests/
│   ├── unit/              # Unity tests (C) — core algorithms
│   ├── integration/       # C++17 tests — pipeline/system
│   └── fixtures/          # Test data (WAV files, configs)
├── benchmarks/            # Performance measurement
├── tools/                 # Python tooling (uv + ty + ruff)
├── docs/                  # Architecture, design decisions
├── cmake/                 # CMake toolchain files & presets
├── meson/                 # Meson cross/native files
├── scripts/               # Build/flash/debug helpers
└── third_party/           # Vendored deps (Unity, etc.)
```

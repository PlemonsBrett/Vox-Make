# Template Quick Start

## 1. Install Dependencies

### C/C++ Toolchain (Desktop)

```bash
# RHEL / Fedora / AlmaLinux / Rocky Linux (dnf)
sudo dnf install gcc gcc-c++ make cmake ninja-build \
    portaudio-devel pkgconf-pkg-config clang-tools-extra

# macOS
brew install cmake ninja portaudio pkg-config clang-format
```

On RHEL, enable **CodeReady Linux Builder** (CRB) or **PowerTools** if `cmake` or other build deps are not found in the default repos. Some packages (for example cross-toolchains) may also need [EPEL](https://docs.fedoraproject.org/en-US/epel/).

### ARM Cross-Compiler (for QEMU / STM32)

```bash
# RHEL / Fedora / AlmaLinux / Rocky Linux (dnf)
sudo dnf install arm-none-eabi-gcc arm-none-eabi-newlib qemu-system-arm

# macOS
brew install arm-none-eabi-gcc qemu
```

### Python Tools

```bash
# Install uv if you haven't
curl -LsSf https://astral.sh/uv/install.sh | sh

# Set up the tools sub-project
cd tools
uv sync --all-extras
cd ..
```

## 2. Build & Test (Desktop)

```bash
# Configure + build + test in one shot
./scripts/build.sh native

# Or step by step:
cmake --preset native
cmake --build --preset native
ctest --preset native
```

## 3. Your First TDD Cycle

```bash
# 1. Open tests/unit/test_ring_buffer.c
# 2. Write a failing test:

void test_ring_buffer_write_advances_head(void) {
    ring_buffer_t buf;
    ring_buffer_init(&buf, 1024);
    TEST_ASSERT_EQUAL(0, ring_buffer_count(&buf));

    int16_t sample = 42;
    ring_buffer_write(&buf, &sample, 1);
    TEST_ASSERT_EQUAL(1, ring_buffer_count(&buf));
}

# 3. Uncomment the RUN_TEST line in run_test_ring_buffer()
# 4. Uncomment run_test_ring_buffer() in test_main.c
# 5. Add test_ring_buffer.c to CMakeLists.txt test_runner sources

# 6. Build — watch it fail (RED)
cmake --build --preset native
ctest --preset native    # FAILS — ring_buffer.h doesn't exist yet

# 7. Create src/core/include/core/ring_buffer.h
# 8. Create src/core/ring_buffer.c with minimal implementation (GREEN)
# 9. Add ring_buffer.c to CMakeLists.txt core library sources
# 10. Build & test — watch it pass
# 11. Refactor if needed — stay green
```

## 4. Cross-Compile for QEMU

```bash
# Build
./scripts/build.sh qemu

# Run in emulator
./scripts/run-qemu.sh

# With GDB debugging
./scripts/run-qemu.sh --gdb
# In another terminal:
arm-none-eabi-gdb -ex 'target remote :1234' build/qemu/vox-wake
```

## 5. Python Tooling

```bash
cd tools

# Generate test audio
uv run generate-test-audio --type sine --freq 440 --duration 2.0 -o ../tests/fixtures/sine_440.wav

# Run Python tests
uv run pytest

# Lint & format
uv run ruff check .
uv run ruff format .

# Type check
uv run ty check
```

## 6. Project Workflow

```plaintext
┌──────────────┐     ┌──────────────┐     ┌──────────────┐
│  Write test  │────→│  Watch FAIL  │────→│ Write code   │
│  (RED)       │     │  (verify)    │     │ (GREEN)      │
└──────────────┘     └──────────────┘     └──────┬───────┘
       ↑                                         │
       │              ┌──────────────┐           │
       └──────────────│  Refactor    │←──────────┘
                      │  (stay green)│
                      └──────────────┘

Desktop (TDD loop):  cmake --build --preset native && ctest --preset native
QEMU (pre-merge):    ./scripts/build.sh qemu
CI (on push):        GitHub Actions runs both + Python tools
Hardware (periodic): Flash to STM32, profile on-device
```

## File Checklist When Adding a New Module

When you add a new DSP module (e.g., `fft`), touch these files:

1. `src/core/include/core/fft.h` — public C header
2. `src/core/fft.c` — implementation
3. `CMakeLists.txt` — add `src/core/fft.c` to `core` library sources
4. `meson.build` — add to `core_sources` list (if using Meson)
5. `tests/unit/test_fft.c` — Unity tests
6. `tests/unit/test_main.c` — add `run_test_fft()` call
7. `CMakeLists.txt` — add `tests/unit/test_fft.c` to `test_runner` sources

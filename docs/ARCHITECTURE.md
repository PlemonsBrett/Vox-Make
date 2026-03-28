# Architecture

## System Overview

```mermaid
graph TB
    subgraph "Application Layer (C++17)"
        CLI[CLI / Config Parser]
        PIPE[Pipeline Orchestrator]
        RAII[RAII Wrappers]
    end

    subgraph "Core DSP (C17)"
        BUF[Ring Buffer]
        WIN[Windowing]
        FFT[FFT]
        MEL[Mel Filterbank]
        MFCC[MFCC Extraction]
        MODEL[Detection / Synthesis Model]
    end

    subgraph "Platform HAL"
        direction LR
        NATIVE[Native\nPortAudio\nclock_gettime]
        QEMU[QEMU\nSemihosting\nDWT Counter]
        STM32[STM32F4\nI2S/SAI + DMA\nDWT Counter]
    end

    subgraph "Python Tools"
        AUDIOGEN[Audio Generator]
        EVAL[Accuracy Evaluator]
        PROF[Memory Profiler]
    end

    CLI --> PIPE
    PIPE --> RAII
    RAII --> BUF
    BUF --> WIN --> FFT --> MEL --> MFCC --> MODEL

    BUF -.->|hal_audio_start callback| NATIVE
    BUF -.->|hal_audio_start callback| QEMU
    BUF -.->|hal_audio_start callback| STM32

    AUDIOGEN -->|test WAVs| BUF
    EVAL -->|accuracy metrics| MODEL
    PROF -->|ELF/map parsing| STM32
```

## Data Flow

Audio flows through the system as a stream of fixed-size frames:

```
Mic / File → [Ring Buffer] → [Window] → [FFT] → [Mel] → [MFCC] → [Model]
              16-bit PCM      float32     complex   float    float    decision
              lock-free       Hann/Hamm   radix-2   26-40    13 coef  DTW/etc
                              20-30ms     in-place   filters
```

**Critical constraint:** Everything from Ring Buffer through Model must run
within one audio buffer period. At 16kHz with 256-sample buffers, that's 16ms.

## Memory Map (Embedded Target)

| Region   | Budget  | Contents                                    |
|----------|---------|---------------------------------------------|
| `.text`  | 128 KB  | Code (core DSP + HAL + minimal app)         |
| `.rodata`| 32 KB   | Mel filter coefficients, templates, strings |
| `.data`  | 4 KB    | Initialized globals                         |
| `.bss`   | 16 KB   | Ring buffer, frame buffers, MFCC state      |
| Stack    | 4 KB    | Call stack (no recursion in hot path)        |
| **Total**| **≤184 KB** | Fits in STM32F4 with 256KB SRAM         |

## Build Targets

```mermaid
graph LR
    SRC[Same Source Code] --> |cmake --preset native| NATIVE[Desktop Binary\nx86_64\nPortAudio + file I/O]
    SRC --> |cmake --preset qemu| QEMU[ARM ELF\nCortex-M4\nSemihosting I/O]
    SRC --> |cmake --preset stm32f4| HW[ARM ELF\nCortex-M4\nI2S + DMA]

    NATIVE --> TEST[Unit Tests\nIntegration Tests\nBenchmarks]
    QEMU --> CI[CI Validation\nBinary Size Check]
    HW --> FLASH[Flash to Board\nOn-Device Profiling]
```

## Directory Responsibilities

| Directory         | Language | Heap OK? | OS Calls? | Purpose                        |
|-------------------|----------|----------|-----------|--------------------------------|
| `src/core/`       | C17      | No       | No        | Algorithms, math, DSP          |
| `src/app/`        | C++17    | Desktop  | Via HAL   | CLI, orchestration, RAII       |
| `src/platform/`   | C17      | No       | Yes       | HAL implementations            |
| `tests/unit/`     | C17      | OK       | OK        | Unity tests for core           |
| `tests/integration/` | C++17 | OK       | OK        | Pipeline-level tests           |
| `tools/`          | Python   | —        | —         | Audio gen, eval, profiling     |

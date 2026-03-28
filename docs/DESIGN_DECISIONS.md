# Design Decisions

This document records architectural decisions and their rationale.
Format follows [ADR (Architecture Decision Records)](https://adr.github.io/).

---

## ADR-001: Mixed C17/C++17 Codebase

**Status:** Accepted  
**Date:** 2026-03-28

**Context:**  
We need both embedded-friendly code (no heap, no exceptions, deterministic timing)
and ergonomic application-layer code (RAII, templates, CLI parsing).

**Decision:**  
Core DSP and algorithms are written in C17. Application layer and test harness
infrastructure use C++17. The boundary is `extern "C"` through `hal.h`.

**Consequences:**  

- Core code compiles with any C17 compiler (including bare-metal ARM GCC)
- Core code is easily auditable for embedded constraints (no hidden allocations)
- Application layer gets RAII, `std::string_view`, structured bindings
- Two language standards to maintain, but the boundary is narrow and explicit

---

## ADR-002: HAL Abstraction for Multi-Target Builds

**Status:** Accepted  
**Date:** 2026-03-28

**Context:**  
We want to develop and test on desktop, validate in QEMU CI, and deploy to
real STM32F4 hardware — all from the same codebase.

**Decision:**  
All platform interaction goes through `platform/hal.h`. Each target (native,
qemu, stm32f4) provides its own implementation. Core code never calls OS or
hardware APIs directly.

**Consequences:**  

- TDD cycle runs entirely on desktop at native speed
- CI can cross-compile and run in QEMU without hardware
- Adding new targets (BeagleBone, Raspberry Pi, new MCU) = one new HAL file
- Slight abstraction overhead, but audio callback path is direct function pointer

---

## ADR-003: Unity Test Framework (Not GoogleTest)

**Status:** Accepted  
**Date:** 2026-03-28

**Context:**  
Core code is C17. GoogleTest is C++ only. We need to test C functions directly
without a C++ wrapper layer.

**Decision:**  
Use ThrowTheSwitch/Unity for all unit tests of C code. If C++ application-layer
tests are needed later, they can use a separate C++ framework.

**Consequences:**  

- Tests are pure C — they compile on the same toolchain as the code under test
- Unity is tiny (~2KB compiled) and can run on embedded targets for on-device testing
- No CMock dependency initially — add it when we need hardware mocking
- Less feature-rich than GoogleTest (no parameterized tests built-in), but adequate

---

## ADR-004: CMake as Primary Build System; Meson for CI Validation

**Status:** Accepted  
**Date:** 2026-03-28

**Context:**  
The repository ships both CMake and Meson so we can compare workflows. Both
support cross-compilation, dependency management, and CI. We need a single
**primary** daily workflow for contributors and tooling, while still proving
the project builds under a second system. A detailed side-by-side analysis lives
in [BUILD_SYSTEM_COMPARISON.md](BUILD_SYSTEM_COMPARISON.md).

**Decision:**  

1. **CMake is the primary build system.** All day-to-day development, presets,
   documentation, and “how to build” instructions assume CMake first.
2. **Developer experience:** Use **CMake + `compile_commands.json` + clangd**
   for navigation, completion, and diagnostics (presets already set
   `CMAKE_EXPORT_COMPILE_COMMANDS=ON`). VS Code CMake Tools / CLion align with
   this path.
3. **Meson remains in the tree** and **CI runs Meson** (or will continue to)
   as a **validation** path — an independent second description of sources,
   flags, and targets so drift or omissions in CMake are more likely to surface.
   Meson is **not** the default workflow for local development or onboarding.

**Consequences:**  

- **Pros:** Strong IDE/LSP story (clangd + CMake); broad editor and toolchain
  integration; extensive public documentation and examples for CMake;
  `CMakePresets.json` keeps multi-target UX consistent; Meson in CI reduces
  single-build-system blind spots.
- **Cons:** Two build definitions must stay in sync when targets, sources, or
  flags change; Meson’s cross-file ergonomics are unused as the main path;
  contributors must know CMake is authoritative when the two disagree.
- **Operational rule:** If CMake and Meson diverge, **CMake wins** until the
  Meson side is updated to match.

---

<!-- Template for new ADRs:

## ADR-NNN: Title

**Status:** Proposed | Accepted | Deprecated | Superseded by ADR-XXX
**Date:** YYYY-MM-DD

**Context:**
What is the issue? Why does it need a decision?

**Decision:**
What was decided? Be specific.

**Consequences:**
What are the trade-offs? What becomes easier? What becomes harder?

-->

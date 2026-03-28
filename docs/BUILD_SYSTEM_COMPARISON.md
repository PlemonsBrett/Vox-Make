# CMake vs Meson — Side-by-Side Comparison for This Template

> Both build systems are wired in this repository. [DESIGN_DECISIONS.md](DESIGN_DECISIONS.md)
> records the project choice (ADR-004: CMake primary, Meson validated in CI).

---

## Adding a New Source File

### CMake

```cmake
# CMakeLists.txt — find the target, add the file
add_library(core STATIC
    src/core/ring_buffer.c
    src/core/fft.c           # ← add here
)
```

You must manually add every `.c` file. Globbing (`file(GLOB ...)`) is
explicitly discouraged by CMake maintainers because it doesn't re-run
configure when new files appear.

### Meson

```meson
# meson.build — same pattern
core_sources = [
    'src/core/ring_buffer.c',
    'src/core/fft.c',           # ← add here
]
```

Same manual listing, but Meson's syntax is cleaner — it's just Python-like
lists instead of CMake's space-separated string weirdness.

**Verdict:** Meson is slightly nicer syntactically, but the workflow is identical.

---

## Cross-Compilation

### CMake

```bash
cmake --preset qemu
# Uses cmake/arm-none-eabi.cmake toolchain file
# Toolchain files use CMAKE_SYSTEM_NAME, CMAKE_C_COMPILER, etc.
# ~40 lines of boilerplate per toolchain
```

CMake toolchain files are well-documented but verbose. The `CMakePresets.json`
layer makes the UX clean, but you're maintaining two levels of config.

### Meson

```bash
meson setup builddir --cross-file meson/arm-qemu.ini
# Cross file is an INI with [binaries], [host_machine], [built-in options]
# ~20 lines per target
```

Meson's cross files are half the size and more readable. Cross-compilation
is a first-class citizen — Meson was designed for it.

**Verdict:** Meson wins on cross-compilation ergonomics. CMake works fine
but requires more boilerplate.

---

## IDE Integration

### CMake

- **VS Code:** CMake Tools extension is excellent — auto-detects presets,
  IntelliSense just works, build/debug/test from the sidebar
- **CLion:** Native CMake support, best-in-class experience
- **compile_commands.json:** Generated automatically with
  `CMAKE_EXPORT_COMPILE_COMMANDS=ON` (already in the template presets)
- **clangd:** Works immediately with compile_commands.json

### Meson

- **VS Code:** Meson extension exists but is less polished than CMake Tools.
  Introspection-based, generally works but occasional rough edges
- **CLion:** Meson support via plugin, not native — noticeably worse than CMake
- **compile_commands.json:** Generated automatically in the build directory
- **clangd:** Works, same as CMake

**Verdict:** CMake wins on IDE support, especially if you use CLion or VS Code
heavily. If you're a terminal-first developer, this matters less.

---

## CI Integration

### CMake

```yaml
- run: cmake --preset native
- run: cmake --build --preset native
- run: ctest --preset native
```

Three clean commands. Presets make CI config trivial.

### Meson

```yaml
- run: meson setup builddir --native-file meson/native.ini
- run: meson compile -C builddir
- run: meson test -C builddir
```

Also three clean commands. Slightly more typing but equally straightforward.

**Verdict:** Tie. Both are clean in CI.

---

## Dependency Management

### CMake

```cmake
# FetchContent for source deps (used for Unity in the template)
FetchContent_Declare(unity
    GIT_REPOSITORY https://github.com/ThrowTheSwitch/Unity.git
    GIT_TAG v2.6.0
)
FetchContent_MakeAvailable(unity)

# find_package for system deps
find_package(PkgConfig)
pkg_check_modules(PORTAUDIO portaudio-2.0)
```

FetchContent works but adds configure-time overhead. For larger deps,
you'll want vcpkg or Conan.

### Meson

```meson
# Wrap system for source deps
# subprojects/unity.wrap — 4 lines of INI
unity_dep = dependency('unity', fallback: ['unity', 'unity_dep'])

# System deps
portaudio_dep = dependency('portaudio-2.0', required: false)
```

Meson's wrap system is cleaner than FetchContent. The `dependency()` function
with fallback is elegant — try system first, fall back to subproject.

**Verdict:** Meson's dependency story is cleaner. CMake's FetchContent works
but feels bolted on.

---

## Documentation and third-party ecosystem

### CMake

- Very large corpus of tutorials, forum answers, and vendor examples assume CMake.
- `CMakePresets.json` is a portable, tool-agnostic way to encode configure/build
  defaults (supported by CMake itself, several IDEs, and CI runners).

### Meson

- Official docs are clear; heavy use in major upstreams (e.g. Mesa, systemd,
  parts of GNOME) demonstrates production maturity.
- Fewer generic “embedded C project” copy-paste examples than CMake overall.

**Verdict:** For “find an answer quickly” across arbitrary toolchains and IDEs,
CMake still has more surface area. Meson is well covered where it is already the
house standard.

---

## Project stance (see ADR-004)

**CMake is the primary build system; Meson stays for CI validation.** Rationale:

1. **IDE/LSP:** CMake integrates tightly with common C/C++ editor workflows
   (e.g. CMake Tools, CLion) and feeds **clangd** cleanly via
   `compile_commands.json` (see [IDE Integration](#ide-integration)).
2. **Presets:** `cmake --preset …` / `cmake --build --preset …` keeps
   configure and build flags consistent without ad hoc shell scripts.
3. **Second build graph in CI:** Running Meson in CI checks that the Meson
   description still matches the project; mistakes that only touch CMake are
   more likely to be caught before they rot the alternate build.

### If you standardize on CMake only

```bash
rm -rf meson/ meson.build meson.options subprojects/
# Remove Meson steps from the CI workflow (e.g. workflows/ci.yml).
```

### If you standardize on Meson only

```bash
rm -rf cmake/ CMakeLists.txt CMakePresets.json third_party/unity/CMakeLists.txt
# Remove CMake steps from the CI workflow.
```

### Current layout (dual system)

No extra steps: develop with CMake locally; CI exercises both. If the two
diverge, **CMake is authoritative** until Meson is updated (ADR-004).

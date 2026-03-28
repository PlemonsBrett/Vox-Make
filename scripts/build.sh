#!/usr/bin/env bash
# scripts/build.sh — Build helper for all targets
#
# Usage:
#   ./scripts/build.sh native       # desktop build (default)
#   ./scripts/build.sh qemu         # QEMU Cortex-M4
#   ./scripts/build.sh stm32f4      # real hardware
#   ./scripts/build.sh all          # all targets
#   ./scripts/build.sh clean        # remove all build dirs

set -euo pipefail

TARGET="${1:-native}"
BUILD_SYSTEM="${BUILD_SYSTEM:-cmake}"   # or "meson"

RED='\033[0;31m'
GREEN='\033[0;32m'
CYAN='\033[0;36m'
NC='\033[0m'

log() { echo -e "${CYAN}[build]${NC} $*"; }
ok()  { echo -e "${GREEN}[  ok ]${NC} $*"; }
err() { echo -e "${RED}[fail]${NC} $*" >&2; }

build_cmake() {
    local target="$1"
    log "CMake: configuring ${target}..."
    cmake --preset "${target}"
    log "CMake: building ${target}..."
    cmake --build --preset "${target}"

    if [[ "${target}" == "native" ]]; then
        log "CMake: running tests..."
        ctest --preset native
    fi

    if [[ "${target}" != "native" ]]; then
        log "Binary size:"
        arm-none-eabi-size "build/${target}/vox-make" 2>/dev/null || true
    fi

    ok "${target} build complete"
}

build_meson() {
    local target="$1"
    local builddir="build-meson-${target}"

    case "${target}" in
        native)   ini_flag="--native-file meson/native.ini" ;;
        qemu)     ini_flag="--cross-file meson/arm-qemu.ini" ;;
        stm32f4)  ini_flag="--cross-file meson/arm-stm32f4.ini" ;;
        *)        err "Unknown target: ${target}"; exit 1 ;;
    esac

    log "Meson: setting up ${target}..."
    # shellcheck disable=SC2086
    meson setup "${builddir}" ${ini_flag} --wipe 2>/dev/null || \
    meson setup "${builddir}" ${ini_flag}

    log "Meson: compiling ${target}..."
    meson compile -C "${builddir}"

    if [[ "${target}" == "native" ]]; then
        log "Meson: running tests..."
        meson test -C "${builddir}"
    fi

    ok "${target} build complete"
}

build() {
    if [[ "${BUILD_SYSTEM}" == "meson" ]]; then
        build_meson "$1"
    else
        build_cmake "$1"
    fi
}

case "${TARGET}" in
    native|qemu|stm32f4)
        build "${TARGET}"
        ;;
    all)
        build native
        build qemu
        build stm32f4
        ;;
    clean)
        log "Cleaning all build directories..."
        rm -rf build/ build-meson-*/
        ok "Clean"
        ;;
    *)
        err "Unknown target: ${TARGET}"
        echo "Usage: $0 {native|qemu|stm32f4|all|clean}"
        exit 1
        ;;
esac

#!/usr/bin/env bash
# scripts/run-qemu.sh — Run firmware in QEMU Cortex-M4 emulator
#
# Usage:
#   ./scripts/run-qemu.sh                        # run default binary
#   ./scripts/run-qemu.sh build/qemu/my_binary   # run specific binary
#   ./scripts/run-qemu.sh --gdb                   # start with GDB server

set -euo pipefail

BINARY="${1:-build/qemu/vox-make}"
GDB_FLAG=""

if [[ "${1:-}" == "--gdb" ]]; then
    GDB_FLAG="-s -S"   # -s = gdbserver on :1234, -S = halt at start
    BINARY="${2:-build/qemu/vox-make}"
    echo "[qemu] GDB server on :1234 — connect with:"
    echo "  arm-none-eabi-gdb -ex 'target remote :1234' ${BINARY}"
fi

if [[ ! -f "${BINARY}" ]]; then
    echo "Binary not found: ${BINARY}"
    echo "Run './scripts/build.sh qemu' first."
    exit 1
fi

echo "[qemu] Running: ${BINARY}"
echo "[qemu] Press Ctrl-A X to exit"
echo "---"

# shellcheck disable=SC2086
qemu-system-arm \
    -machine lm3s6965evb \
    -cpu cortex-m4 \
    -semihosting \
    -nographic \
    -kernel "${BINARY}" \
    ${GDB_FLAG}

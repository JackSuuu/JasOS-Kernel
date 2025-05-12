#!/bin/bash

# This script is for piping QEMU output to a file for later inspection.

KERNEL_IMAGE="kernel.bin"
LOG_FILE="qemu_output.log"

if [ ! -f "$KERNEL_IMAGE" ]; then
    echo "Error: Kernel image '$KERNEL_IMAGE' not found. Build the kernel first."
    exit 1
fi

echo "Running QEMU and logging to '$LOG_FILE'..."

qemu-system-arm \
  -machine versatilepb \
  -cpu arm1176 \
  -chardev stdio,id=char0 -serial chardev:char0 \
  -kernel "$KERNEL_IMAGE" \
  -d int,cpu_reset,unimp,guest_errors \
  > "$LOG_FILE" 2>&1

QEMU_EXIT_CODE=$?

if [ $QEMU_EXIT_CODE -eq 0 ]; then
    echo "QEMU finished successfully. Log saved to '$LOG_FILE'."
else
    echo "QEMU exited with error code $QEMU_EXIT_CODE. Log saved to '$LOG_FILE'."
    echo "Check '$LOG_FILE' for details."
fi

exit $QEMU_EXIT_CODE

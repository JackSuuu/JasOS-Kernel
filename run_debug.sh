#!/bin/bash
# Run JasOS in text-only mode with debug logging

echo "==================================================="
echo "JasOS Kernel - DEBUG Mode"
echo "==================================================="

# Check if kernel.bin exists
if [ ! -f "kernel.bin" ]; then
    echo "Error: kernel.bin not found!"
    echo "Please run 'make' to build the kernel first."
    exit 1
fi

# Create logs directory if it doesn't exist
mkdir -p logs

echo "Starting QEMU with debug logging..."
echo "Press Ctrl+A then X to exit QEMU."
echo

# Run QEMU with detailed logging of all activities
qemu-system-arm -M versatilepb -m 128M -kernel kernel.bin -nographic \
    -d unimp,guest_errors,in_asm,cpu,int,exec,pcall \
    -D logs/qemu_debug.log \
    -serial file:logs/uart_output.log \
    -monitor stdio

echo
echo "QEMU session ended."
echo "Debug logs are available in the logs directory."

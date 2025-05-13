#!/bin/bash
# Run JasOS in text-only mode without graphics

echo "==================================================="
echo "JasOS Kernel - UART Mode"
echo "==================================================="
echo

# Check if kernel.bin exists
if [ ! -f "kernel.bin" ]; then
    echo "Error: kernel.bin not found!"
    echo "Please run 'make' to build the kernel first."
    exit 1
fi

echo "Starting QEMU..."
echo "Press Ctrl+A then X to exit QEMU."
echo

# Run QEMU with proper serial port configuration
qemu-system-arm -M versatilepb -m 128M -kernel kernel.bin -nographic -serial mon:stdio

echo
echo "QEMU session ended."

#!/bin/bash
# Build and run the simplified kernel

echo "===== Building Simplified Kernel ====="
make -f Makefile.simple clean
make -f Makefile.simple

if [ ! -f "kernel_simple.bin" ]; then
    echo "Error: kernel_simple.bin not found!"
    echo "Build failed."
    exit 1
fi

echo
echo "===== Running Simplified Kernel ====="
echo "Press Ctrl+A then X to exit QEMU."
echo

# Run QEMU with basic parameters for UART
qemu-system-arm -M versatilepb -m 128M -kernel kernel_simple.bin -nographic

echo
echo "QEMU session ended." 
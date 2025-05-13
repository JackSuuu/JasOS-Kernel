#!/bin/bash
# Run JasOS with special focus on UART input

echo "==================================================="
echo "JasOS Kernel - UART Input Test"
echo "==================================================="

# Build the UART test kernel
echo "Building UART test kernel..."
make -f Makefile.uart_test

# Check if kernel_uart_test.bin exists
if [ ! -f "kernel_uart_test.bin" ]; then
    echo "Error: kernel_uart_test.bin not found!"
    echo "Build failed."
    exit 1
fi

echo "Starting QEMU with special UART configuration..."
echo "Press Ctrl+A then X to exit QEMU."
echo

# Run QEMU with direct UART connection
# Important: Use mon:stdio for proper input handling
qemu-system-arm -M versatilepb -m 128M -kernel kernel_uart_test.bin \
    -nographic -serial mon:stdio

echo
echo "QEMU session ended."

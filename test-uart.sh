#!/bin/bash
# Build and run UART test

# Make sure we're in the right directory
cd "$(dirname "$0")"

echo "Building UART test..."
arm-none-eabi-g++ -mcpu=arm1176jzf-s -ffreestanding -nostdlib -fno-exceptions -fno-rtti \
    -Wl,--section-start=.text=0x10000 -e _start \
    src/uart-test.cpp -o uart-test.elf

arm-none-eabi-objcopy -O binary uart-test.elf uart-test.bin

echo "Running UART test..."
qemu-system-arm -machine versatilepb -cpu arm1176 -serial stdio -kernel uart-test.bin

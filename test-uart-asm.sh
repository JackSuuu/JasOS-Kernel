#!/bin/bash
# Build and run ASM UART test

# Make sure we're in the right directory
cd "$(dirname "$0")"

echo "Assembling UART test..."
arm-none-eabi-as -mcpu=arm1176jzf-s src/uart-asm-test.s -o uart-asm-test.o
arm-none-eabi-ld -Ttext=0x10000 uart-asm-test.o -o uart-asm-test.elf
arm-none-eabi-objcopy -O binary uart-asm-test.elf uart-asm-test.bin

echo "Running UART assembly test..."
qemu-system-arm -machine versatilepb -cpu arm1176 -serial stdio -kernel uart-asm-test.bin

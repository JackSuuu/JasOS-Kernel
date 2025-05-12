#!/bin/bash
# Debug run script for JSOS with alternative QEMU settings

# Make sure we're in the right directory
cd "$(dirname "$0")"

echo "Running kernel with debug settings..."

qemu-system-arm -machine versatilepb -cpu arm1176 \
                -serial stdio \
                -kernel kernel.bin \
                -d unimp,guest_errors

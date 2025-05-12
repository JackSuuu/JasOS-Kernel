#!/bin/bash
# Run script that uses proven QEMU settings

# Make sure we're in the right directory
cd "$(dirname "$0")"

echo "Running JasOS kernel..."

exec qemu-system-arm \
  -machine versatilepb \
  -cpu arm1176 \
  -serial stdio \
  -kernel kernel.bin

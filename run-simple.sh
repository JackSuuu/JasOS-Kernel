#!/bin/bash
# Run script for JSOS with simplifed QEMU settings

# Make sure we're in the right directory
cd "$(dirname "$0")"

# Try with the simplest QEMU settings
qemu-system-arm \
  -machine versatilepb \
  -serial stdio \
  -kernel kernel.bin

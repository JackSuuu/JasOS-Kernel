#!/bin/bash
# Build script for JSOS

# Make sure we're in the right directory
cd "$(dirname "$0")"

# Clean previous build
make clean

# Build the kernel
make

# Run the kernel if requested
if [ "$1" == "run" ]; then
    make run
fi 
# JSOS - Minimal ARM Kernel OS

A simple operating system kernel for ARM architecture.

## Project Structure

All files are now organized in a flat structure:
- `src/` - Contains all source code files (.cpp, .s) and header files (.hpp)
- `build/` - Generated during build, contains object files
- `Makefile` - Build configuration
- `kernel.bin` - Final binary output

## Building

To build the project, simply run:

```bash
make
```

## Running

To run the OS in QEMU emulator:

```bash
make run
```

## Features

- Simple UART-based console
- Basic task scheduler
- Timer interrupts
- Memory allocation

## Development Notes

- The kernel starts at `_start` in vector.s
- The C++ entry point is `_start_cpp` in kernel.cpp
- The system uses a flat memory model with a simple memory allocator 
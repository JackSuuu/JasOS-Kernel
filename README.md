# JasOS Kernel

**JasOS** Kernel is a Unix-based, bare-metal operating system kernel developed to explore the inner workings of operating systems and understand the complex mechanisms behind them. This project provides a hands-on experience with OS development on ARM architecture.

## Running the Kernel

To run the kernel in a QEMU emulator, use the following command:

```shell
qemu-system-arm -machine versatilepb -cpu arm1176 -nographic -kernel kernel.bin
```

or

```shell
make run
```

### Terminating the Emulator

To stop the emulator, use the following key sequence:

```shell
# Ctrl + A → X
```

## Technology Stack

- **ARM Cross-Compiler**: Utilizes `arm-none-eabi-g++`, ideal for bare-metal development on ARM architecture.
- **QEMU**: Emulates ARM hardware with `qemu-system-arm` to test and run the kernel.

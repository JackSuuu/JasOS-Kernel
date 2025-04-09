# JsOS-Kernel

It is a Unix based bare-metal OS kernel which I developed for delve into a depth of understanding how operating system work and the complex mechanism behind it.

## Run the kernel

```shell
qemu-system-arm -machine versatilepb -cpu arm1176 -nographic -kernel kernel.bin
```

To terminate the emulator:

```shell
# Key sequence: Ctrl+A → X
```

## Technology

- the ARM cross-compiler (arm-none-eabi-g++), which is suitable for bare-metal development.
- qemu-system-arms

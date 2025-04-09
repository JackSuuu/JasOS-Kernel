# Compile and run the kernel
CC = arm-none-eabi-g++
CFLAGS = -nostdlib -ffreestanding -nostartfiles -mcpu=arm1176jzf-s -c
LDFLAGS = -nostdlib -ffreestanding -nostartfiles -T linker.ld

all: kernel.bin

kernel.o: kernel.cpp
	$(CC) $(CFLAGS) $< -o $@

kernel.elf: kernel.o linker.ld
	$(CC) $(LDFLAGS) kernel.o -o $@

kernel.bin: kernel.elf
	arm-none-eabi-objcopy -O binary $< $@

run: kernel.bin
	qemu-system-arm -machine versatilepb -cpu arm1176 -nographic -kernel kernel.bin

clean:
	rm -f kernel.o kernel.elf kernel.bin
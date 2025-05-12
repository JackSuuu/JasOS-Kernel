CC = arm-none-eabi-g++
AS = arm-none-eabi-as
LD = arm-none-eabi-ld
OBJCOPY = arm-none-eabi-objcopy

SRC_DIR = src
BUILD_DIR = build

CFLAGS = -mcpu=arm1176jzf-s -I$(SRC_DIR) -ffreestanding -nostdlib -Wall -fno-exceptions -fno-rtti
ASFLAGS = -mcpu=arm1176jzf-s
LDFLAGS = -nostdlib -T $(SRC_DIR)/linker.ld

# Exclude test files from the build
CPP_SRCS = $(filter-out $(SRC_DIR)/uart-test.cpp, $(wildcard $(SRC_DIR)/*.cpp))
ASM_SRCS = $(filter-out $(SRC_DIR)/uart-asm-test.s, $(wildcard $(SRC_DIR)/*.s))
OBJS = $(patsubst $(SRC_DIR)/%.cpp,$(BUILD_DIR)/%.o,$(CPP_SRCS)) \
       $(patsubst $(SRC_DIR)/%.s,$(BUILD_DIR)/%.o,$(ASM_SRCS))
DEPS = $(OBJS:.o=.d)

.PHONY: all clean run

all: kernel.bin

kernel.bin: kernel.elf
	$(OBJCOPY) -O binary $< $@

kernel.elf: $(OBJS)
	$(LD) $(LDFLAGS) -o $@ $^

$(BUILD_DIR)/%.o: $(SRC_DIR)/%.cpp
	@mkdir -p $(BUILD_DIR)
	$(CC) $(CFLAGS) -MMD -MP -c $< -o $@

$(BUILD_DIR)/%.o: $(SRC_DIR)/%.s
	@mkdir -p $(BUILD_DIR)
	$(AS) $(ASFLAGS) $< -o $@

clean:
	rm -rf $(BUILD_DIR) kernel.elf kernel.bin

run: kernel.bin
	qemu-system-arm -machine versatilepb -cpu arm1176 -chardev stdio,id=char0 -serial chardev:char0 -kernel kernel.bin -d int,cpu_reset,unimp,guest_errors

-include $(DEPS)
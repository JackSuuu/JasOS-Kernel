CC = arm-none-eabi-g++
AS = arm-none-eabi-as
LD = arm-none-eabi-ld
OBJCOPY = arm-none-eabi-objcopy

SRC_DIR = src
BUILD_DIR = build
INCLUDE_DIR = include

CFLAGS = -mcpu=arm1176jzf-s -I$(INCLUDE_DIR) -ffreestanding -nostdlib -Wall
ASFLAGS = -mcpu=arm1176jzf-s
LDFLAGS = -nostdlib -T linker.ld

SRCS = $(wildcard $(SRC_DIR)/*.cpp) $(wildcard $(SRC_DIR)/arch/*.s)
OBJS = $(patsubst $(SRC_DIR)/%, $(BUILD_DIR)/%, $(SRCS:.cpp=.o))
OBJS := $(OBJS:.s=.o)
DEPS = $(OBJS:.o=.d)

.PHONY: all clean run

all: kernel.bin

kernel.bin: kernel.elf
	$(OBJCOPY) -O binary $< $@

kernel.elf: $(OBJS)
	$(CC) $(CFLAGS) $(LDFLAGS) -o $@ $^

$(BUILD_DIR)/%.o: $(SRC_DIR)/%.cpp
	@mkdir -p $(@D)
	$(CC) $(CFLAGS) -MMD -MP -c $< -o $@

$(BUILD_DIR)/arch/%.o: $(SRC_DIR)/arch/%.s
	@mkdir -p $(@D)
	$(AS) $(ASFLAGS) $< -o $@

clean:
	rm -rf $(BUILD_DIR) kernel.elf kernel.bin

run: kernel.bin
	qemu-system-arm -machine versatilepb -cpu arm1176 -nographic -kernel kernel.bin

-include $(DEPS)
# Tools and flags
CC = i686-elf-gcc
LD = i686-elf-ld
NASM = nasm
CFLAGS = -ffreestanding -m32 -g -c
LDFLAGS = -Ttext 0x1000 --oformat binary
QEMU = qemu-system-x86_64

# File targets
SRC_DIR = .
BUILD_DIR = build

KERNEL_SRC = $(SRC_DIR)/kernel.cpp
ENTRY_SRC = $(SRC_DIR)/kernel_entry.asm
BOOTLOADER_SRC = $(SRC_DIR)/bootloader.asm

KERNEL_OBJ = $(BUILD_DIR)/kernel.o
ENTRY_OBJ = $(BUILD_DIR)/kernel_entry.o
KERNEL_BIN = $(BUILD_DIR)/kernel.bin
BOOTLOADER_BIN = $(BUILD_DIR)/bootloader.bin
OS_BIN = $(BUILD_DIR)/os-image.bin

all: $(OS_BIN)
	@echo "Build complete. Run with: make run"

$(KERNEL_OBJ): $(KERNEL_SRC)
	$(CC) $(CFLAGS) $< -o $@

$(ENTRY_OBJ): $(ENTRY_SRC)
	$(NASM) $< -f elf -o $@

$(KERNEL_BIN): $(ENTRY_OBJ) $(KERNEL_OBJ)
	$(LD) $(LDFLAGS) -o $@ $(ENTRY_OBJ) $(KERNEL_OBJ)

$(BOOTLOADER_BIN): $(BOOTLOADER_SRC)
	$(NASM) $< -f bin -o $@

$(OS_BIN): $(BOOTLOADER_BIN) $(KERNEL_BIN)
	cat $^ > $@

run: $(OS_BIN)
	$(QEMU) -drive format=raw,file=$<

clean:
	rm -f $(BUILD_DIR)/*.o $(BUILD_DIR)/*.bin

.PHONY: all run clean

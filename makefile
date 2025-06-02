# Tools and flags
CC = i686-elf-gcc
LD = i686-elf-ld
NASM = nasm
CFLAGS = -ffreestanding -m32 -g -c
LDFLAGS = -Ttext 0x1000 --oformat binary
QEMU = qemu-system-x86_64

# Directories
SRC_DIR = .
BUILD_DIR = build

# Source files
KERNEL_SRC = $(SRC_DIR)/kernel.cpp
ENTRY_SRC = $(SRC_DIR)/kernel_entry.asm
BOOTLOADER_SRC = $(SRC_DIR)/bootloader.asm

# Object and binary files
KERNEL_OBJ = $(BUILD_DIR)/kernel.o
ENTRY_OBJ = $(BUILD_DIR)/kernel_entry.o
KERNEL_BIN = $(BUILD_DIR)/kernel.bin
BOOTLOADER_BIN = $(BUILD_DIR)/bootloader.bin
OS_BIN = $(BUILD_DIR)/os-image.bin

# Build all: OS image
all: $(OS_BIN)
	@echo "Build complete. Run with: make run"

# Compile kernel C++ source to object
$(KERNEL_OBJ): $(KERNEL_SRC)
	$(CC) $(CFLAGS) $< -o $@

# Assemble kernel entry ASM to object
$(ENTRY_OBJ): $(ENTRY_SRC)
	$(NASM) $< -f elf -o $@

# Link kernel objects into flat binary
$(KERNEL_BIN): $(ENTRY_OBJ) $(KERNEL_OBJ)
	$(LD) $(LDFLAGS) -o $@ $(ENTRY_OBJ) $(KERNEL_OBJ)

# Assemble bootloader to flat binary
$(BOOTLOADER_BIN): $(BOOTLOADER_SRC)
	$(NASM) $< -f bin -o $@

# Concatenate bootloader + kernel to form disk image
$(OS_BIN): $(BOOTLOADER_BIN) $(KERNEL_BIN)
	cat $^ > $@

# Run QEMU using floppy drive (-fda) for correct disk emulation
run: $(OS_BIN)
	$(QEMU) -fda $<

# Clean build files
clean:
	rm -f $(BUILD_DIR)/*.o $(BUILD_DIR)/*.bin

.PHONY: all run clean

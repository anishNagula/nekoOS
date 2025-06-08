# Tools and flags
CC = i686-elf-gcc
LD = i686-elf-ld
NASM = nasm
CFLAGS = -ffreestanding -m32 -g -c -Isrc
LDFLAGS = -Ttext 0x1000 --oformat binary
QEMU = qemu-system-x86_64

# Directories
SRC_DIR = src
BUILD_DIR = build

# Source files
CPP_SOURCES = $(SRC_DIR)/kernel.cpp $(SRC_DIR)/vga.cpp $(SRC_DIR)/io.cpp $(SRC_DIR)/keyboard.cpp $(SRC_DIR)/command.cpp $(SRC_DIR)/utils.cpp
ASM_SOURCES = $(SRC_DIR)/kernel_entry.asm $(SRC_DIR)/bootloader.asm

# Object files (for all .cpp + kernel_entry.asm only)
OBJ_FILES = $(patsubst $(SRC_DIR)/%.cpp,$(BUILD_DIR)/%.o,$(CPP_SOURCES)) \
            $(BUILD_DIR)/kernel_entry.o

# Binaries
KERNEL_BIN = $(BUILD_DIR)/kernel.bin
BOOTLOADER_BIN = $(BUILD_DIR)/bootloader.bin
OS_BIN = $(BUILD_DIR)/os-image.bin

# Default target
all: $(OS_BIN)
	@echo "Build complete. Run with: make run"

# Compile .cpp files
$(BUILD_DIR)/%.o: $(SRC_DIR)/%.cpp
	@mkdir -p $(BUILD_DIR)
	$(CC) $(CFLAGS) -o $@ $<

# Assemble kernel entry (ELF)
$(BUILD_DIR)/kernel_entry.o: $(SRC_DIR)/kernel_entry.asm
	$(NASM) $< -f elf -o $@

# Assemble bootloader (binary)
$(BOOTLOADER_BIN): $(SRC_DIR)/bootloader.asm
	$(NASM) $< -f bin -o $@

# Link kernel ELF .o files into flat binary
$(KERNEL_BIN): $(OBJ_FILES)
	$(LD) $(LDFLAGS) -o $@ $(OBJ_FILES)

# Combine bootloader and kernel binary to form final image
$(OS_BIN): $(BOOTLOADER_BIN) $(KERNEL_BIN)
	cat $^ > $@

# Run in QEMU
run: $(OS_BIN)
	$(QEMU) -fda $<

# Clean build files
clean:
	rm -f $(BUILD_DIR)/*.o $(BUILD_DIR)/*.bin

.PHONY: all run clean

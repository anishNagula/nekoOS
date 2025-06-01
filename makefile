# Define tools and flags
CC = i686-elf-gcc
LD = i686-elf-ld
NASM = nasm
CFLAGS = -ffreestanding -m32 -g -c
LDFLAGS = -Ttext 0x1000 --oformat binary
QEMU = qemu-system-x86_64

# File targets
KERNEL_C = kernel.cpp
KERNEL_OBJ = kernel.o
ENTRY_ASM = kernel_entry.asm
ENTRY_OBJ = kernel_entry.o
KEYBOARD_ASM = keyboard.asm
KEYBOARD_OBJ = keyboard.o
KERNEL_BIN = full_kernel.bin
BOOTLOADER_ASM = bootloader.asm
BOOTLOADER_BIN = bootloader.bin
ZEROES_ASM = zeroes.asm
ZEROES_BIN = zeroes.bin
EVERYTHING_BIN = everything.bin
OS_BIN = OS.bin

all: $(OS_BIN)
	@echo "Build complete. Run with: make run"

$(KERNEL_OBJ): $(KERNEL_C)
	$(CC) $(CFLAGS) $< -o $@

$(ENTRY_OBJ): $(ENTRY_ASM)
	$(NASM) $< -f elf -o $@

$(KEYBOARD_OBJ): $(KEYBOARD_ASM)
	$(NASM) $< -f elf -o $@

$(KERNEL_BIN): $(ENTRY_OBJ) $(KERNEL_OBJ) $(KEYBOARD_OBJ)
	$(LD) $(LDFLAGS) -o $@ $(ENTRY_OBJ) $(KERNEL_OBJ) $(KEYBOARD_OBJ)

$(BOOTLOADER_BIN): $(BOOTLOADER_ASM)
	$(NASM) $< -f bin -o $@

$(ZEROES_BIN): $(ZEROES_ASM)
	$(NASM) $< -f bin -o $@

$(EVERYTHING_BIN): $(BOOTLOADER_BIN) $(KERNEL_BIN)
	cat $(BOOTLOADER_BIN) $(KERNEL_BIN) > $@

$(OS_BIN): $(EVERYTHING_BIN) $(ZEROES_BIN)
	cat $(EVERYTHING_BIN) $(ZEROES_BIN) > $@

run: $(OS_BIN)
	$(QEMU) -drive format=raw,file=$(OS_BIN)

clean:
	rm -f *.o *.bin

.PHONY: all run clean

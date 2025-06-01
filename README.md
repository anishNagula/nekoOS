# NekoOS

**NekoOS** is a minimalist 32-bit operating system built from scratch using C and Assembly for the x86 architecture. It includes a custom bootloader, kernel with direct VGA text-mode output, and low-level keyboard input via scancodes. The project serves as an educational introduction to operating system fundamentals, protected mode, and bare-metal programming.

---

## Features

- 🧠 **Bootloader**: Custom bootloader in x86 Assembly that loads the kernel into memory.
- 🛡 **Protected Mode**: Transition from real mode to 32-bit protected mode using GDT.
- ⌨️ **Keyboard Input**: Raw scancode reading and ASCII translation.
- 📺 **VGA Text Output**: Direct writing to memory-mapped VGA buffer (0xB8000).
- ⛔ **ESC Detection**: Exits input loop gracefully on pressing the ESC key.
- 🕹 **Backspace Handling**: Simple backspace implementation that clears characters.

---

## Build & Run

### Prerequisites

- `nasm`
- `i686-elf-gcc`
- `i686-elf-ld`
- `qemu` (for emulation)

You can install these using your system's package manager. For example, on Ubuntu:

```bash
sudo apt install nasm qemu gcc binutils
```

You may also need to build a cross-compiler for `i686-elf-gcc`. [Refer to the OSDev wiki](https://wiki.osdev.org/GCC_Cross-Compiler).

### Build

```bash
make
```

### Run in QEMU

```bash
make run
```

---

## Project Structure

```plaintext
.
├── bootloader.asm        # Custom bootloader (loads kernel)
├── kernel.cpp            # Main kernel logic (keyboard input, VGA output)
├── kernel_entry.asm      # Entry point in protected mode
├── Makefile              # Build configuration
├── zeroes.asm            # Padding to ensure 512-byte alignment
└── README.md
```

---

## Keyboard Input Mapping

- Scancodes from port `0x60` are read directly.
- Mapped to ASCII using a custom lookup table.
- ESC key (`0x01`) exits the loop.
- Backspace (`0x0E`) clears the previous character visually and logically.

---

## Goals

This project aims to:
- Understand how real and protected mode differ.
- Explore memory-mapped I/O and direct hardware access.
- Write a basic text UI without relying on external libraries.
- Lay the foundation for adding features like:
  - Paging / memory management
  - File system support
  - System calls
  - Shell interface

---

## License

MIT License – use, modify, and learn freely!

---

## Credits

- Inspired by the [OSDev.org](https://wiki.osdev.org/) community and tutorials.
- Named "NekoOS" after the Japanese word for cat (猫), symbolizing curiosity and lightness.

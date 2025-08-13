# NekoOS

**NekoOS** is a minimal 32-bit operating system built using C and Assembly for the x86 architecture. It includes a custom bootloader, kernel with direct VGA text-mode output, and keyboard input via scancodes.

---

## Features

- **Bootloader**: Bootloader in x86 Assembly that loads the custom kernel into memory.
- **Protected Mode**: Transition from real mode to 32-bit protected mode using GDT.
- **Keyboard Input**: Raw scancode reading and ASCII translation.
- **VGA Text Output**: Direct writing to memory mapped VGA buffer (0xB8000).
- **ESC Detection**: Exits input loop on pressing the ESC key.
- **Backspace Handling**: Simple backspace implementation that clears characters.

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
├── src/
│   ├── bootloader.asm
│   ├── kernel.cpp
│   ├── kernel_entry.asm
|   ├── command.cpp
|   ├── vga.cpp
|   ├── io.cpp
|   ├── keyboard.cpp
|   └── utils.cpp
├── Makefile
└── README.md
```

---

## Keyboard Input Mapping

- Scancodes from port `0x60` are read directly.
- Mapped to ASCII using a custom lookup table.
- ESC key (`0x01`) exits the loop.
- Backspace (`0x0E`) clears the previous character visually and logically.

---

## Future workd
- PIT
- Paging / memory management
- File system support
- System calls
- Shell interface

---

## License

MIT License

---

## Credits

- References to [OSDev.org](https://wiki.osdev.org/) community and tutorials.
- Named "NekoOS" after the Japanese word for cat (猫), symbolizing curiosity and lightness.

#include "vga.h"
#include "io.h"

// === vga cursor update ===
void move_cursor(int cursor) {
    u16 pos = cursor / 2;
    outb(0x3D4, 0x0F);
    outb(0x3D5, (u8)(pos & 0xFF));
    outb(0x3D4, 0x0E);
    outb(0x3D5, (u8)((pos >> 8) & 0xFF));
}

// === write character to vga ===
void write_vga_char(volatile char* vga, int cursor, char c, bool is_title) {
    if (cursor % 2 != 0) cursor--;  // Ensure alignment
    vga[cursor] = c;
    vga[cursor + 1] = is_title ? TITLE_COLOR : WHITE_ON_BLACK;
}

// === scroll screen up ===
void scroll(volatile char* vga, int* cursor) {
    for (int i = 0; i < (VGA_HEIGHT - 1) * VGA_WIDTH; ++i) {
        vga[i * 2] = vga[(i + VGA_WIDTH) * 2];
        vga[i * 2 + 1] = vga[(i + VGA_WIDTH) * 2 + 1];
    }

    for (int i = (VGA_HEIGHT - 1) * VGA_WIDTH; i < VGA_HEIGHT * VGA_WIDTH; ++i) {
        vga[i * 2] = ' ';
        vga[i * 2 + 1] = WHITE_ON_BLACK;
    }

    *cursor -= VGA_WIDTH * CHAR_WIDTH;
    if (*cursor < 0) *cursor = 0;
}

// === print a full line ===
void print_line(const char* msg, volatile char* vga, int* cursor, bool is_title) {
    for (int i = 0; msg[i] != '\0'; ++i) {
        if (*cursor >= SCREEN_SIZE) scroll(vga, cursor);
        write_vga_char(vga, *cursor, msg[i], is_title);
        *cursor += 2;
    }

}

// === typewriter effect for welcome title ===
void print_line_typewriter(const char* msg, volatile char* vga, int* cursor, bool is_title) {
    for (int i = 0; msg[i] != '\0'; ++i) {
        if (*cursor >= SCREEN_SIZE) scroll(vga, cursor);
        write_vga_char(vga, *cursor, msg[i], is_title);
        *cursor += 2;
        move_cursor(*cursor);

        delay(9000000);
    }

    *cursor = ((*cursor / (VGA_WIDTH * CHAR_WIDTH)) + 1) * (VGA_WIDTH * CHAR_WIDTH);
    move_cursor(*cursor);
}

// === to clear the screen ===
void clear_screen(volatile char* vga, int* cursor) {
    for (int i = 0; i < VGA_WIDTH * VGA_HEIGHT; ++i) {   // fill all screen cells with blank character
        vga[i * CHAR_WIDTH]     = ' ';
        vga[i * CHAR_WIDTH + 1] = WHITE_ON_BLACK;
    }
    // setting the cursor to start/0
    *cursor = 0;
    move_cursor(*cursor);
}
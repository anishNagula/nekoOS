#pragma once
#include <stdbool.h>

#define VGA_WIDTH 80
#define VGA_HEIGHT 25
#define CHAR_WIDTH 2
#define SCREEN_SIZE (VGA_WIDTH * VGA_HEIGHT * CHAR_WIDTH)
#define WHITE_ON_BLACK 0x0F
#define TITLE_COLOR 0x0A

void move_cursor(int cursor);
void write_vga_char(volatile char* vga, int cursor, char c, bool is_title);
void scroll(volatile char* vga, int* cursor);
void print_line(const char* msg, volatile char* vga, int* cursor, bool is_title);
void print_line_typewriter(const char* msg, volatile char* vga, int* cursor, bool is_title);
void clear_screen(volatile char* vga, int* cursor);

#pragma once
#include "io.h"

#define MAX_INPUT 128
#define KB_DATA_READY (inb(0x64) & 1)

extern char input_buffer[MAX_INPUT];
extern int input_buffer_index;
extern bool shift_pressed;

char scancode_to_ascii(u8 scancode);
void handle_scancode(u8 scancode, volatile char* vga, int* cursor);
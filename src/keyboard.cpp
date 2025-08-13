#include "keyboard.h"
#include "vga.h"
#include "command.h"
#include "io.h"

char input_buffer[MAX_INPUT];
int  input_buffer_index = 0;
bool shift_pressed = false;

// scancode to ascii convert func
char scancode_to_ascii(u8 scancode) {
    static const char normal_map[0x54] = {
        0,    0,    '1',  '2',  '3',  '4',  '5',  '6',
        '7',  '8',  '9',  '0',  '-',  '=',  '\b', '\t',
        'q',  'w',  'e',  'r',  't',  'y',  'u',  'i',
        'o',  'p',  '[',  ']',  '\n', 0,    'a',  's',
        'd',  'f',  'g',  'h',  'j',  'k',  'l',  ';',
        '\'', '`',  0,    '\\', 'z',  'x',  'c',  'v',
        'b',  'n',  'm',  ',',  '.',  '/',  0,    '*',
        0,  ' ',    0,    0,    0,    0,    0,    0,
        0,    0,    0,    0,    0,    0,    0,    '7',
        '8',  '9',  '-',  '4',  '5',  '6',  '+',  '1',
        '2',  '3',  '0',  '.'
    };

    static const char shift_map[0x54] = {
        0,    0,    '!',  '@',  '#',  '$',  '%',  '^',
        '&',  '*',  '(',  ')',  '_',  '+',  '\b', '\t',
        'Q',  'W',  'E',  'R',  'T',  'Y',  'U',  'I',
        'O',  'P',  '{',  '}',  '\n', 0,    'A',  'S',
        'D',  'F',  'G',  'H',  'J',  'K',  'L',  ':',
        '"',  '~',  0,    '|',  'Z',  'X',  'C',  'V',
        'B',  'N',  'M',  '<',  '>',  '?',  0,    '*',
        ' ',  0,    0,    0,    0,    0,    0,    0,
        0,    0,    0,    0,    0,    0,    0,    '7',
        '8',  '9',  '-',  '4',  '5',  '6',  '+',  '1',
        '2',  '3',  '0',  '.'
    };

    if (scancode >= 0x54) return 0;
    return shift_pressed ? shift_map[scancode] : normal_map[scancode];
}

// handle key press
void handle_scancode(u8 scancode, volatile char* vga, int* cursor) {
    if (scancode & 0x80) {
        u8 released = scancode & 0x7F;
        if (released == 0x2A || released == 0x36) {
            shift_pressed = false;
        }
        return;
    }

    // if shift
    if (scancode == 0x2A || scancode == 0x36) {
        shift_pressed = true;
        return;
    }

    if (scancode == 0x01) {   // escape
        *cursor = ((*cursor / (VGA_WIDTH * CHAR_WIDTH)) + 1) * (VGA_WIDTH * CHAR_WIDTH);
        print_line("ESC pressed. Quitting...", vga, cursor, false);
        while (1) asm volatile("hlt");
    }
    else if (scancode == 0x0E && input_buffer_index > 0) {     // backspace
        *cursor -= 2;
        write_vga_char(vga, *cursor, ' ', false);
        move_cursor(*cursor);
        input_buffer[--input_buffer_index] = '\0';
    }
    else if (scancode == 0x1C) {         // enter 
        if (input_buffer_index > 0) {
            input_buffer[input_buffer_index] = '\0';
            parse_and_execute(input_buffer, vga, cursor);
        }
        input_buffer_index = 0;
        input_buffer[0] = '\0';

        *cursor = ((*cursor / (VGA_WIDTH * CHAR_WIDTH)) + 1) * VGA_WIDTH * CHAR_WIDTH;
        move_cursor(*cursor);
        write_vga_char(vga, *cursor, '>', false);
        *cursor += 2;
        move_cursor(*cursor);
    }
    else {       // any other key
        char ch = scancode_to_ascii(scancode);
        if (ch) {
            if (*cursor >= SCREEN_SIZE) scroll(vga, cursor);
            input_buffer[input_buffer_index++] = ch;
            write_vga_char(vga, *cursor, ch, false);
            *cursor += 2;
            move_cursor(*cursor);
        }
    }
}
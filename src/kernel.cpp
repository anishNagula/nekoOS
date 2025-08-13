#include "vga.h"
#include "keyboard.h"
#include "command.h"


extern "C" void main() {
    volatile char* vga = (char*)0xB8000;
    int cursor = 0;

    print_line_typewriter("Welcome to NekoOS", vga, &cursor, true);

    write_vga_char(vga, cursor, '>', false);
    cursor+=2;
    move_cursor(cursor);

    while (true) {
        while (!KB_DATA_READY) {}
        handle_scancode(inb(0x60), vga, &cursor);
    }
}

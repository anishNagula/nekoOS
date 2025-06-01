// reading from port
unsigned char inb(unsigned short port) {
    unsigned char result;
    asm volatile ("inb %1, %0" : "=a"(result) : "Nd"(port));
    return result;
}

// writing to a port
void outb(unsigned short port, unsigned char data) {
    asm volatile ("outb %0, %1" : : "a"(data), "Nd"(port));
}

char scancode_to_ascii(unsigned char scancode) {
    const char map[58] = {
        '?',  '?',  '1',  '2',  '3',  '4',  '5',  '6',  '7',  '8',
        '9',  '0',  '-',  '=',  '\b', '\t', 'q',  'w',  'e',  'r',
        't',  'y',  'u',  'i',  'o',  'p',  '[',  ']',  '\n', '?',
        'a',  's',  'd',  'f',  'g',  'h',  'j',  'k',  'l',  ';',
        '\'', '`',  '?',  '\\', 'z',  'x',  'c',  'v',  'b',  'n',
        'm',  ',',  '.',  '/',  '?',  '?',  ' '
    };
    if (scancode >= 58) return '?';
    return map[scancode];
}

void move_cursor(int cursor) {
    unsigned short pos = cursor / 2;
    outb(0x3D4, 0x0F);
    outb(0x3D5, (unsigned char)(pos & 0xFF));
    outb(0x3D4, 0x0E);
    outb(0x3D5, (unsigned char)((pos >> 8) & 0xFF));
}

void scroll(char* vga, int* cursor) {
    for (int i = 0; i < 24 * 80 * 2; ++i) {
        vga[i] = vga[i + 160];
    }

    // clear the last row for new line
    for (int i = 24 * 160; i < 25 * 160; i += 2) {
        vga[i] = ' ';
        vga[i + 1] = 0x0F;
    }

    *cursor -= 160;
}

void scroll_if_needed(char* vga, int* cursor) {
    if (*cursor >= 80 * 25 * 2) {
        scroll(vga, cursor);
    }
}

void print_char(char c) {
    volatile char* vga = (char*)0xB8000;
    *vga++ = c;
    *vga++ = 0x0F;
}

extern "C" void main() {
    volatile char* vga = (char*)0xB8000;
    int cursor = 0;

    while (true) {
        unsigned char status;

        // wait for key press event to occur
        do {
            status = inb(0x64);
        } while (!(status & 1));

        unsigned char scancode = inb(0x60);

        // ecs key press
        if (scancode == 0x01) {
            cursor = (cursor / 160 + 1) * 160;
            scroll_if_needed((char*)vga, &cursor);
            move_cursor(cursor);

            const char* msg = "ESC pressed. Stopping input.";
            for (int i = 0; msg[i]; ++i) {
                vga[cursor++] = msg[i];
                vga[cursor++] = 0x0F;
            }
            break;
        }

        // backspace key press
        else if (scancode == 0x0E && cursor > 0) {
            cursor -= 2;
            vga[cursor] = ' ';
            vga[cursor + 1] = 0x0F;
            move_cursor(cursor);
            continue;
        }

        // enter key press
        else if (scancode == 0x1C) {
            cursor = (cursor / 160 + 1) * 160;
            scroll_if_needed((char*)vga, &cursor);
            move_cursor(cursor);
            continue;
        }

        char ch = scancode_to_ascii(scancode);
        if (ch == '?') continue;

        vga[cursor++] = ch;
        vga[cursor++] = 0x0F;
        scroll_if_needed((char*)vga, &cursor);
        move_cursor(cursor);
    }

    // halt after exiting
    while (1) {
        asm volatile ("hlt");
    }
}

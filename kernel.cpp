#define VGA_WIDTH 80
#define VGA_HEIGHT 25
#define CHAR_WIDTH 2
#define SCREEN_SIZE (VGA_WIDTH * VGA_HEIGHT * CHAR_WIDTH)
#define WHITE_ON_BLACK 0x0F

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
    static const char map[58] = {
        '?',  '?',  '1',  '2',  '3',  '4',  '5',  '6',  '7',  '8',
        '9',  '0',  '-',  '=',  '\b', '\t', 'q',  'w',  'e',  'r',
        't',  'y',  'u',  'i',  'o',  'p',  '[',  ']',  '\n', '?',
        'a',  's',  'd',  'f',  'g',  'h',  'j',  'k',  'l',  ';',
        '\'', '`',  '?',  '\\', 'z',  'x',  'c',  'v',  'b',  'n',
        'm',  ',',  '.',  '/',  '?',  '?',  ' '
    };
    return (scancode < 58) ? map[scancode] : '?';
}

void move_cursor(int cursor) {
    unsigned short pos = cursor / 2;
    outb(0x3D4, 0x0F);
    outb(0x3D5, (unsigned char)(pos & 0xFF));
    outb(0x3D4, 0x0E);
    outb(0x3D5, (unsigned char)((pos >> 8) & 0xFF));
}

void write_vga_char(volatile char* vga, int cursor, char c) {
    vga[cursor] = c;
    vga[cursor + 1] = WHITE_ON_BLACK;
}

void scroll(volatile char* vga, int* cursor) {
    for (int i = 0; i < (VGA_HEIGHT - 1) * VGA_WIDTH * CHAR_WIDTH; ++i) {
        vga[i] = vga[i + VGA_WIDTH * CHAR_WIDTH];
    }

    for (int i = (VGA_HEIGHT - 1) * VGA_WIDTH * CHAR_WIDTH;
         i < VGA_HEIGHT * VGA_WIDTH * CHAR_WIDTH;
         i += 2) {
        vga[i] = ' ';
        vga[i + 1] = WHITE_ON_BLACK;
    }

    *cursor -= VGA_WIDTH * CHAR_WIDTH;
}

void scroll_if_needed(volatile char* vga, int* cursor) {
    if (*cursor >= SCREEN_SIZE) {
        scroll(vga, cursor);
    }
}

void print_line(const char* msg, volatile char* vga, int* cursor) {

    for (int i = 0; msg[i] != '\0'; ++i) {
        write_vga_char(vga, *cursor, msg[i]);
        *cursor += 2;
    }

    *cursor = ((*cursor / (VGA_WIDTH * CHAR_WIDTH)) + 1) * (VGA_WIDTH * CHAR_WIDTH);
    move_cursor(*cursor);
}

extern "C" void main() {
    volatile char* vga = (char*)0xB8000;
    int cursor = 0;

    print_line("Welcome to NekoOS!", vga, &cursor);

    while (true) {
        // wait for key press
        while (!(inb(0x64) & 1)) {}

        unsigned char scancode = inb(0x60);

        // ESC to exit
        if (scancode == 0x01) {
            print_line("ESC pressed. Stopping input.", vga, &cursor);
            break;
        }

        // backspace
        else if (scancode == 0x0E && cursor >= 2) {
            cursor -= 2;
            write_vga_char(vga, cursor, ' ');
            move_cursor(cursor);
            continue;
        }

        // enter key
        else if (scancode == 0x1C) {
            cursor = ((cursor / (VGA_WIDTH * CHAR_WIDTH)) + 1) * VGA_WIDTH * CHAR_WIDTH;
            scroll_if_needed(vga, &cursor);
            move_cursor(cursor);
            continue;
        }

        char ch = scancode_to_ascii(scancode);
        if (ch == '?') continue;

        write_vga_char(vga, cursor, ch);
        cursor += 2;
        scroll_if_needed(vga, &cursor);
        move_cursor(cursor);
    }

    while (1) {
        asm volatile ("hlt");
    }
}

#define VGA_WIDTH 80
#define VGA_HEIGHT 25
#define CHAR_WIDTH 2
#define SCREEN_SIZE (VGA_WIDTH * VGA_HEIGHT * CHAR_WIDTH)
#define WHITE_ON_BLACK 0x0F
#define TITLE_COLOR 0x0A

#define KB_DATA_READY (inb(0x64) & 1)

typedef unsigned char  u8;
typedef unsigned short u16;

// === I/O Ports ===
u8 inb(u16 port) {
    u8 result;
    asm volatile ("inb %1, %0" : "=a"(result) : "Nd"(port));
    return result;
}

void outb(u16 port, u8 data) {
    asm volatile ("outb %0, %1" : : "a"(data), "Nd"(port));
}

// === Scancode to ASCII map ===
char scancode_to_ascii(u8 scancode) {
    static const char map[58] = {
        '?',  '?',  '1',  '2',  '3',  '4',  '5',  '6',  '7',  '8',
        '9',  '0',  '-',  '=',  '\b', '\t', 'q',  'w',  'e',  'r',
        't',  'y',  'u',  'i',  'o',  'p',  '[',  ']',  '\n', '?',
        'a',  's',  'd',  'f',  'g',  'h',  'j',  'k',  'l',  ';',
        '\'', '`',  '?',  '\\', 'z',  'x',  'c',  'v',  'b',  'n',
        'm',  ',',  '.',  '/',  '?',  '?',  ' '
    };
    return (scancode < sizeof(map)) ? map[scancode] : '?';
}

// === VGA cursor update ===
void move_cursor(int cursor) {
    u16 pos = cursor / 2;
    outb(0x3D4, 0x0F);
    outb(0x3D5, (u8)(pos & 0xFF));
    outb(0x3D4, 0x0E);
    outb(0x3D5, (u8)((pos >> 8) & 0xFF));
}

// === Write character to VGA ===
void write_vga_char(volatile char* vga, int cursor, char c, bool is_title) {
    if (cursor % 2 != 0) cursor--;  // Ensure alignment
    vga[cursor] = c;
    vga[cursor + 1] = is_title ? TITLE_COLOR : WHITE_ON_BLACK;
}

// === Scroll screen up ===
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

// === Print a full line ===
void print_line(const char* msg, volatile char* vga, int* cursor, bool is_title) {
    for (int i = 0; msg[i] != '\0'; ++i) {
        if (*cursor >= SCREEN_SIZE) scroll(vga, cursor);
        write_vga_char(vga, *cursor, msg[i], is_title);
        *cursor += 2;
    }

    *cursor = ((*cursor / (VGA_WIDTH * CHAR_WIDTH)) + 1) * (VGA_WIDTH * CHAR_WIDTH);
    move_cursor(*cursor);
}

// Simple delay loop (tune the count to adjust speed)
void delay(int count) {
    for (volatile int i = 0; i < count; i++) {
        // Do nothing, just burn time
        asm volatile("nop");
    }
}

// Print a full line with typewriter effect
void print_line_typewriter(const char* msg, volatile char* vga, int* cursor, bool is_title) {
    for (int i = 0; msg[i] != '\0'; ++i) {
        if (*cursor >= SCREEN_SIZE) scroll(vga, cursor);
        write_vga_char(vga, *cursor, msg[i], is_title);
        *cursor += 2;
        move_cursor(*cursor);

        delay(10000000);  // Adjust delay count here for speed (higher = slower)
    }

    *cursor = ((*cursor / (VGA_WIDTH * CHAR_WIDTH)) + 1) * (VGA_WIDTH * CHAR_WIDTH);
    move_cursor(*cursor);
}

// === Keyboard event handler ===
void handle_scancode(u8 scancode, volatile char* vga, int* cursor) {
    if (scancode & 0x80) return; // Ignore key release

    if (scancode == 0x01) {
        // Move cursor to the next line start
        *cursor = ((*cursor / (VGA_WIDTH * CHAR_WIDTH)) + 1) * (VGA_WIDTH * CHAR_WIDTH);
        print_line("ESC pressed. Stopping input.", vga, cursor, false);
        while (1) asm volatile("hlt");
    }
    else if (scancode == 0x0E && *cursor >= 2) {
        *cursor -= 2;
        write_vga_char(vga, *cursor, ' ', false);
        move_cursor(*cursor);
    }
    else if (scancode == 0x1C) {
        *cursor = ((*cursor / (VGA_WIDTH * CHAR_WIDTH)) + 1) * VGA_WIDTH * CHAR_WIDTH;
        move_cursor(*cursor);
    }
    else {
        char ch = scancode_to_ascii(scancode);
        if (ch != '?') {
            if (*cursor >= SCREEN_SIZE) scroll(vga, cursor);
            write_vga_char(vga, *cursor, ch, false);
            *cursor += 2;
            move_cursor(*cursor);
        }
    }
}

// === Kernel main ===
extern "C" void main() {
    volatile char* vga = (char*)0xB8000;
    int cursor = 0;

    print_line_typewriter("Welcome to NekoOS!", vga, &cursor, true);

    while (true) {
        while (!KB_DATA_READY) {}
        handle_scancode(inb(0x60), vga, &cursor);
    }
}

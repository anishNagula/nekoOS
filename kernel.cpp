#define VGA_WIDTH 80
#define VGA_HEIGHT 25
#define CHAR_WIDTH 2
#define SCREEN_SIZE (VGA_WIDTH * VGA_HEIGHT * CHAR_WIDTH)
#define WHITE_ON_BLACK 0x0F
#define TITLE_COLOR 0x0A

#define KB_DATA_READY (inb(0x64) & 1)
#define MAX_INPUT 128

char input_buffer[MAX_INPUT];
int  input_buffer_index = 0;

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

// === scancode to ASCII converter ===
char scancode_to_ascii(u8 scancode) {
    static const char map[60] = {
        '?',  '?',  '1',  '2',  '3',  '4',  '5',  '6',  '7',  '8',
        '9',  '0',  '-',  '=',  '\b', '\t', 'q',  'w',  'e',  'r',
        't',  'y',  'u',  'i',  'o',  'p',  '[',  ']',  '\n', '?',
        'a',  's',  'd',  'f',  'g',  'h',  'j',  'k',  'l',  ';',
        '\'', '`',  '?',  '\\', 'z',  'x',  'c',  'v',  'b',  'n',
        'm',  ',',  '.',  '/',  '?',  '?',  '?',  ' '  // <- 0x39 is last
    };
    return (scancode < sizeof(map)) ? map[scancode] : '?';
}

// === qemu shutdown ===
static void shutdown() {
    outb(0x604, 0x00);
    outb(0x605, 0x20);
}

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

// delay loop
void delay(int count) {
    for (volatile int i = 0; i < count; i++) {
        asm volatile("nop");
    }
}

// typewriter effect for welcome title
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

void clear_screen(volatile char* vga, int* cursor) {
    for (int i = 0; i < VGA_WIDTH * VGA_HEIGHT; ++i) {   // fill all screen cells with blank character
        vga[i * CHAR_WIDTH]     = ' ';
        vga[i * CHAR_WIDTH + 1] = WHITE_ON_BLACK;
    }
    // setting the cursor to start/0
    *cursor = 0;
    move_cursor(*cursor);
}


void parse_and_execute(char* input_buffer, volatile char* vga, int* cursor) {
    // trimming whitespace in start and end
    int start = 0;
    while (input_buffer[start] == ' ') start++;

    int end = start;
    while (input_buffer[end] != '\0') end++;
    end--;

    while (end >= start && input_buffer[end] == ' ') end--;

    int trimmed_length = end - start + 1;
    if (trimmed_length <= 0) {
        print_line("> ", vga, cursor, false);
        return;
    }


    // checking for help command
    if (trimmed_length == 4 &&
        input_buffer[start] == 'h' &&
        input_buffer[start + 1] == 'e' &&
        input_buffer[start + 2] == 'l' &&
        input_buffer[start + 3] == 'p') {
        *cursor = ((*cursor / (VGA_WIDTH * CHAR_WIDTH)) + 1) * (VGA_WIDTH * CHAR_WIDTH);
        move_cursor(*cursor);
        print_line("Available commands: help, clear, echo, reboot", vga, cursor, false);
        return;
    }

    // checking for clear command
    if (trimmed_length == 5 &&
        input_buffer[start] == 'c' &&
        input_buffer[start + 1] == 'l' &&
        input_buffer[start + 2] == 'e' &&
        input_buffer[start + 3] == 'a' &&
        input_buffer[start + 4] == 'r') {
        clear_screen(vga, cursor);
        return;
    }

    // checking for reboot command
    if (trimmed_length == 6 &&
        input_buffer[start] == 'r' &&
        input_buffer[start + 1] == 'e' &&
        input_buffer[start + 2] == 'b' &&
        input_buffer[start + 3] == 'o' &&
        input_buffer[start + 4] == 'o' &&
        input_buffer[start + 5] == 't') {
        *cursor = ((*cursor / (VGA_WIDTH * CHAR_WIDTH)) + 1) * (VGA_WIDTH * CHAR_WIDTH);
        move_cursor(*cursor);
        print_line("Reboot not implemented yet", vga, cursor, false);
        return;
    }

    // checking for exit command
    if (trimmed_length == 4 &&
        input_buffer[start] == 'e' &&
        input_buffer[start+1] == 'x' &&
        input_buffer[start+2] == 'i' &&
        input_buffer[start+3] == 't' ) {
        *cursor = ((*cursor / (VGA_WIDTH * CHAR_WIDTH)) + 1) * (VGA_WIDTH * CHAR_WIDTH);
        move_cursor(*cursor);
        shutdown();
    }

    // checking for echo message
    if (trimmed_length >= 4 &&
        input_buffer[start] == 'e' &&
        input_buffer[start + 1] == 'c' &&
        input_buffer[start + 2] == 'h' &&
        input_buffer[start + 3] == 'o') {

        // checking if only echo is typed
        if (trimmed_length == 4) {
            print_line("", vga, cursor, false);
            return;
        }

        if (input_buffer[start + 4] != ' ') {
            print_line("Unknown command", vga, cursor, false);
            return;
        }

        int msg_start = start + 5;
        char message[120];
        int i = 0;
        while (msg_start <= end && i < 119) {
            message[i++] = input_buffer[msg_start++];
        }
        message[i] = '\0';
        *cursor = ((*cursor / (VGA_WIDTH * CHAR_WIDTH)) + 1) * (VGA_WIDTH * CHAR_WIDTH);
        move_cursor(*cursor);
        print_line(message, vga, cursor, false);
        return;
    }

    // every other case
    *cursor = ((*cursor / (VGA_WIDTH * CHAR_WIDTH)) + 1) * (VGA_WIDTH * CHAR_WIDTH);
    move_cursor(*cursor);
    print_line("Unknown command", vga, cursor, false);
}

void handle_scancode(u8 scancode, volatile char* vga, int* cursor) {

    if (scancode & 0x80) return;

    if (scancode == 0x01) {   // escape key
        *cursor = ((*cursor / (VGA_WIDTH * CHAR_WIDTH)) + 1) * (VGA_WIDTH * CHAR_WIDTH);
        print_line("ESC pressed. Quitting...", vga, cursor, false);
        while (1) asm volatile("hlt");
    }
    else if (scancode == 0x0E && *cursor >= 2) {     // backspace key
        *cursor -= 2;
        write_vga_char(vga, *cursor, ' ', false);
        move_cursor(*cursor);
        input_buffer[--input_buffer_index] = '\0';
    }
    else if (scancode == 0x1C) {         // enter key

        if (input_buffer_index > 0) {
            input_buffer[input_buffer_index] = '\0';
            parse_and_execute(input_buffer, vga, cursor);
        }

        input_buffer_index = 0;
        input_buffer[0] = '\0';

        
        *cursor = ((*cursor / (VGA_WIDTH * CHAR_WIDTH)) + 1) * VGA_WIDTH * CHAR_WIDTH;
        move_cursor(*cursor);
        write_vga_char(vga, *cursor, '>', false);
        *cursor+=2;
        move_cursor(*cursor);
        write_vga_char(vga, *cursor, ' ', false);
        *cursor+=2;
        move_cursor(*cursor);
    }
    else {       // all other keys
        char ch = scancode_to_ascii(scancode);
        if (ch != '?') {
            if (*cursor >= SCREEN_SIZE) scroll(vga, cursor);
            input_buffer[input_buffer_index++] = ch;
            write_vga_char(vga, *cursor, ch, false);
            *cursor += 2;
            move_cursor(*cursor);
        }
    }
}


// === kernel main ===
extern "C" void main() {
    volatile char* vga = (char*)0xB8000;
    int cursor = 0;

    print_line_typewriter("Welcome to NekoOS", vga, &cursor, true);

    write_vga_char(vga, cursor, '>', false);
    cursor+=2;
    move_cursor(cursor);
    write_vga_char(vga, cursor, ' ', false);
    cursor+=2;
    move_cursor(cursor);

    while (true) {
        while (!KB_DATA_READY) {}
        handle_scancode(inb(0x60), vga, &cursor);
    }
}

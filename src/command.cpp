#include "command.h"
#include "vga.h"
#include "io.h"
#include "utils.h"

#ifndef NULL
#define NULL 0
#endif

typedef void (*command_func_t)(volatile char* vga, int* cursor, char* args);

typedef struct {
    const char* name;
    int length;
    command_func_t func;
} command_t;


// === Command Handlers ===
void cmd_help(volatile char* vga, int* cursor, char* args) {
    *cursor = ((*cursor / (VGA_WIDTH * CHAR_WIDTH)) + 1) * (VGA_WIDTH * CHAR_WIDTH);
    move_cursor(*cursor);
    print_line("Available commands: help, clear, echo, reboot, exit", vga, cursor, false);
}

void cmd_clear(volatile char* vga, int* cursor, char* args) {
    clear_screen(vga, cursor);
}

void cmd_reboot(volatile char* vga, int* cursor, char* args) {
    *cursor = ((*cursor / (VGA_WIDTH * CHAR_WIDTH)) + 1) * (VGA_WIDTH * CHAR_WIDTH);
    move_cursor(*cursor);
    print_line("Reboot not implemented yet", vga, cursor, false);
}

void cmd_exit(volatile char* vga, int* cursor, char* args) {
    *cursor = ((*cursor / (VGA_WIDTH * CHAR_WIDTH)) + 1) * (VGA_WIDTH * CHAR_WIDTH);
    move_cursor(*cursor);
    shutdown(vga, cursor);
}

void cmd_echo(volatile char* vga, int* cursor, char* args) {
    if (!args) {
        print_line("", vga, cursor, false);
        return;
    }
    *cursor = ((*cursor / (VGA_WIDTH * CHAR_WIDTH)) + 1) * (VGA_WIDTH * CHAR_WIDTH);
    move_cursor(*cursor);
    print_line(args, vga, cursor, false);
}


// === Command List ===
command_t commands[] = {
    {"help", 4, cmd_help},
    {"clear", 5, cmd_clear},
    {"reboot", 6, cmd_reboot},
    {"exit", 4, cmd_exit},
    {"echo", 4, cmd_echo},
};

int num_commands = sizeof(commands) / sizeof(command_t);


// === Main parse and execute function ===
void parse_and_execute(char* input_buffer, volatile char* vga, int* cursor) {
    int start = 0;
    int length = 0;

    while (input_buffer[length] != '\0') length++;

    trim_whitespace(input_buffer, &start, &length);

    if (length <= 0) {
        print_line("> ", vga, cursor, false);
        return;
    }

    for (int i = 0; i < num_commands; i++) {
        if (length >= commands[i].length) {
            if (my_strcmp(&input_buffer[start], commands[i].name, commands[i].length) == 0) {
                char* args = NULL;
                // for echo command
                if (commands[i].name[0] == 'e' && commands[i].name[1] == 'c') {
                    if (length == commands[i].length) {
                        args = NULL;
                    } else if (input_buffer[start + commands[i].length] == ' ') {
                        args = &input_buffer[start + commands[i].length + 1];
                    } else {
                        break;
                    }
                }
                commands[i].func(vga, cursor, args);
                return;
            }
        }
    }

    // for unknown command
    *cursor = ((*cursor / (VGA_WIDTH * CHAR_WIDTH)) + 1) * (VGA_WIDTH * CHAR_WIDTH);
    move_cursor(*cursor);
    print_line("Unknown command", vga, cursor, false);
}


// === shutdown ===
void shutdown(volatile char* vga, int* cursor) {
    print_line_typewriter("Shutting down...", vga, cursor, true);

    outb(0x604, 0x00);
    outb(0x605, 0x20);
}

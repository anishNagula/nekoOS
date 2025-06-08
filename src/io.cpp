#include "io.h"

// === i/o ports ===
u8 inb(u16 port) {
    u8 result;
    asm volatile ("inb %1, %0" : "=a"(result) : "Nd"(port));
    return result;
}

void outb(u16 port, u8 data) {
    asm volatile ("outb %0, %1" : : "a"(data), "Nd"(port));
}

// === delay loop ===
void delay(int count) {
    for (volatile int i = 0; i < count; i++) {
        asm volatile("nop");
    }
}
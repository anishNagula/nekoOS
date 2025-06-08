#pragma once

typedef unsigned char  u8;
typedef unsigned short u16;

u8 inb(u16 port);
void outb(u16 port, u8 data);
void delay(int count);
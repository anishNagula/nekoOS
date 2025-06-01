; keyboard.asm
[bits 32]
section .text
global read_scancode

read_scancode:
    ; Wait for key press
.wait:
    in al, 0x64
    test al, 1
    jz .wait

    ; Read the scancode
    in al, 0x60
    ret

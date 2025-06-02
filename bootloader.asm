[org 0x7c00]                        
KERNEL_LOCATION equ 0x1000

mov [BOOT_DISK], dl

xor ax, ax
mov es, ax            ; ES=0 for int 0x13 buffer segment
mov ds, ax
mov bp, 0x8000
mov sp, bp

mov bx, KERNEL_LOCATION
mov ah, 0x02          ; Read sectors
mov al, 20            ; Number of sectors to read
mov ch, 0x00          ; Cylinder 0
mov dh, 0x00          ; Head 0
mov cl, 0x02          ; Sector 2 (after boot sector)
mov dl, [BOOT_DISK]
int 0x13              ; BIOS read sectors

mov ah, 0x0
mov al, 0x3
int 0x10              ; Set text mode

CODE_SEG equ 0x08
DATA_SEG equ 0x10

cli
lgdt [GDT_descriptor]
mov eax, cr0
or eax, 1
mov cr0, eax
jmp CODE_SEG:start_protected_mode

jmp $

BOOT_DISK: db 0

GDT_start:
    GDT_null:
        dd 0x0
        dd 0x0

    GDT_code:
        dw 0xffff
        dw 0x0
        db 0x0
        db 0b10011010
        db 0b11001111
        db 0x0

    GDT_data:
        dw 0xffff
        dw 0x0
        db 0x0
        db 0b10010010
        db 0b11001111
        db 0x0

GDT_end:

GDT_descriptor:
    dw GDT_end - GDT_start - 1
    dd GDT_start


[bits 32]
start_protected_mode:
    mov ax, DATA_SEG
    mov ds, ax
    mov ss, ax
    mov es, ax
    mov fs, ax
    mov gs, ax

    mov ebp, 0x90000
    mov esp, ebp


    jmp CODE_SEG:0x1000

jump_message: db "Loading Kernel..."

times 510-($-$$) db 0
dw 0xAA55

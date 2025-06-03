[org 0x7c00]

KERNEL_LOCATION equ 0x1000

mov [BOOT_DISK], dl        ; Save boot disk number

; printing "Loading Kernel..." message before loading kernel
mov si, jump_message
.print_char_msg:
    lodsb
    cmp al, 0
    je .done_print
    mov ah, 0x0E
    int 0x10
    jmp .print_char_msg
.done_print:

; intentional delay loop
mov cx, 0xFFFF
.delay_outer:
    push cx
    mov cx, 0x4000
.delay_inner:
    loop .delay_inner
    pop cx
loop .delay_outer

xor ax, ax
mov es, ax                ; ES=0 for int 0x13 buffer segment
mov ds, ax
mov bp, 0x8000
mov sp, bp

mov bx, KERNEL_LOCATION
mov ah, 0x02              ; BIOS read sectors function
mov al, 20                ; Number of sectors to read (32)
mov ch, 0x00              ; Cylinder 0
mov dh, 0x00              ; Head 0
mov cl, 0x02              ; Sector 2 (after boot sector)
mov dl, [BOOT_DISK]
int 0x13
jc disk_error             ; Jump if carry flag set (error)

mov ah, 0x00
mov al, 0x03
int 0x10

CODE_SEG equ 0x08
DATA_SEG equ 0x10

cli
lgdt [GDT_descriptor]

mov eax, cr0
or eax, 1                ; Set PE bit to enter protected mode
mov cr0, eax

jmp CODE_SEG:start_protected_mode

jmp $                    ; Infinite loop if something goes wrong

; === Disk error handler ===
disk_error:
    mov bl, ah           ; BIOS error code in BL

    mov si, disk_error_msg
.print_char:
    lodsb
    cmp al, 0
    je .print_code
    mov ah, 0x0E
    int 0x10
    jmp .print_char

.print_code:
    ; Print high nibble of BL
    mov al, bl
    shr al, 4
    call print_hex_nibble

    ; Print low nibble of BL
    mov al, bl
    and al, 0x0F
    call print_hex_nibble

    jmp $                ; Halt here forever

print_hex_nibble:
    cmp al, 9
    jbe .digit
    add al, 'A' - 10
    jmp .print
.digit:
    add al, '0'
.print:
    mov ah, 0x0E
    int 0x10
    ret

; === GDT ===
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

; === Protected mode start ===
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

    jmp CODE_SEG:0x1000     ; Jump to kernel start

jump_message: db "Loading Kernel...", 0

disk_error_msg db "Disk error: 0x", 0

times 510-($-$$) db 0
dw 0xAA55

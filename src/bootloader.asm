[org 0x7c00]

KERNEL_LOCATION equ 0x1000

mov [BOOT_DISK], dl        ; save the boot disk number


mov si, jump_message
.print_char_msg:
    lodsb
    cmp al, 0
    je .done_print
    mov ah, 0x0E
    int 0x10
    jmp .print_char_msg
.done_print:

; forced loop
mov cx, 0xFFFF
.delay_outer:
    push cx
    mov cx, 0x4000
.delay_inner:
    loop .delay_inner
    pop cx
loop .delay_outer

xor ax, ax
mov es, ax                
mov ds, ax
mov bp, 0x8000
mov sp, bp

mov bx, KERNEL_LOCATION
mov ah, 0x02              ; BIOS func to read sectors
mov al, 20                ; no.of sectors to read [32]
mov ch, 0x00              ; cylinder 0
mov dh, 0x00              ; head 0
mov cl, 0x02              ; sector2
mov dl, [BOOT_DISK]
int 0x13
jc disk_error             ; jump if error (settig carry flag)

mov ah, 0x00
mov al, 0x03
int 0x10

CODE_SEG equ 0x08
DATA_SEG equ 0x10

cli
lgdt [GDT_descriptor]

mov eax, cr0
or eax, 1                ; entering protected mode
mov cr0, eax

jmp CODE_SEG:start_protected_mode

jmp $                    ; loop on error

; code to handle disk error
disk_error:
    mov bl, ah
    mov si, disk_error_msg

.print_char:
    lodsb
    cmp al, 0
    je .print_code
    mov ah, 0x0E
    int 0x10
    jmp .print_char

.print_code:
    mov al, bl
    shr al, 4
    call print_hex_nibble

    mov al, bl
    and al, 0x0F
    call print_hex_nibble

    jmp $                ; halt infinightly

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

; GDT
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


; protected mode code
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

    jmp CODE_SEG:0x1000     ; jump to kernel start

jump_message: db "Loading Kernel...", 0

disk_error_msg db "Disk error: 0x", 0

times 510-($-$$) db 0
dw 0xAA55

;=============================================
; MAXX-NANO BOOT SECTOR
; Pure DOS-style boot - no Windows
; ================================================================
[BITS 16]
[ORG 0x7C00]

start:
    cli
    xor ax, ax
    mov ds, ax
    mov es, ax
    mov ss, ax
    mov sp, 0x7C00
    
    mov si, boot_msg
    call print
    
    ; Load stage2 (16 sectors = 8KB)
    mov bx, 0x8000
    mov ah, 0x02
    mov al, 16
    mov ch, 0
    mov cl, 2
    mov dh, 0
    mov dl, 0x80
    int 0x13
    
    jmp 0x0000:0x8000

print:
    lodsb
    or al, al
    jz .done
    mov ah, 0x0E
    int 0x10
    jmp print
.done:
    ret

boot_msg: db 0x0D, 0x0A, "MAXX-NANO v0.03 - POD CORE", 0x0D, 0x0A
          db "==============================", 0x0D, 0x0A, 0

times 510-($-$$) db 0
dw 0xAA55

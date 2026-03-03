;=============================================
; MAXX-NANO BOOT SECTOR
; Automation shell - NOT an LLM
; Fits in boot sector (512 bytes)
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
    
    ; Load stage2 (8 sectors)
    mov bx, 0x8000
    mov ah, 0x02
    mov al, 8
    mov ch, 0
    mov cl, 2
    mov dh, 0
    mov dl, 0x80
    int 0x13
    
    jmp 0x0000:0x8000

times 510-($-$$) db 0
dw 0xAA55

;=============================================
; MAXX BOOT SECTOR - Fits in 1.44MB floppy
; Target: 1440KB total boot image
;=============================================
[BITS 16]
[ORG 0x7C00]

; ═══════════════════════════════════════════════════════════════
; MAXX BOOT LOADER - The smallest bootable AI gateway
; Size target: < 15KB for boot sector + stage2
; ================================================================

start:
    cli
    xor ax, ax
    mov ds, ax
    mov es, ax
    mov ss, ax
    mov sp, 0x7C00
    
    ; Print boot message
    mov si, boot_msg
    call print_str
    
    ; Load stage2 (8 sectors = 4KB) to 0x8000
    mov bx, 0x8000
    mov ah, 0x02
    mov al, 8
    mov ch, 0
    mov cl, 2
    mov dh, 0
    mov dl, 0x00    ; First floppy/hdd
    int 0x13
    jc disk_error
    
    ; Load MAXX kernel to 0x10000 (64KB after boot sector area)
    mov bx, 0x10000
    mov ah, 0x02
    mov al, 64      ; 32KB kernel
    mov ch, 0
    mov cl, 10
    mov dh, 0
    mov dl, 0x00
    int 0x13
    
    ; Jump to stage2
    jmp 0x0000:0x8000

disk_error:
    mov si, err_msg
    call print_str
    jmp $

print_str:
    lodsb
    or al, al
    jz .done
    mov ah, 0x0E
    int 0x10
    jmp print_str
.done:
    ret

boot_msg: db "MAXX v0.01 - Booting to consciousness...", 0x0D, 0x0A, 0
err_msg:   db "MAXX: Boot error - halting", 0x0D, 0x0A, 0

times 510-($-$$) db 0
dw 0xAA55

; ================================================================
; END OF BOOT SECTOR (512 bytes)
; Remaining space for stage2 in sectors 2-9 (4KB)
; ================================================================

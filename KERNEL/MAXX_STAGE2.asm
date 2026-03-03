;=============================================
; MAXX STAGE2 + KERNEL BOOTSTRAP
; Combined: ~8KB total (fits in sectors 2-9)
; ================================================================
[BITS 16]
[ORG 0x8000]

stage2_start:
    ; Print loading message
    mov si, stage2_msg
    call print
    
    ; Enable A20 line (required for protected mode)
    in al, 0x92
    or al, 2
    out 0x92, al
    
    ; Setup GDT for protected mode
    lgdt [gdt_descriptor]
    
    ; Enable protected mode
    mov eax, cr0
    or eax, 1
    mov cr0, eax
    
    ; Far jump to 32-bit mode
    jmp 0x08:protected_mode

[BITS 32]
protected_mode:
    ; Setup segment registers
    mov ax, 0x10
    mov ds, ax
    mov es, ax
    mov fs, ax
    mov gs, ax
    mov ss, ax
    
    ; Setup stack
    mov esp, 0x90000
    
    ; Copy kernel to final location
    mov esi, 0x10000      ; Source (where we loaded kernel)
    mov edi, 0x00100000  ; Destination (1MB - standard kernel loc)
    mov ecx, 8192        ; 8KB kernel
    rep movsb
    
    ; Jump to MAXX kernel entry
    jmp kernel_entry

; ================================================================
; GDT (Global Descriptor Table)
; ================================================================
gdt_start:
    dq 0x0000000000000000    ; Null descriptor
    dq 0x00CF9A000000FFFF    ; Code segment (4GB, 32-bit)
    dq 0x00CF92000000FFFF    ; Data segment (4GB, 32-bit)
gdt_end:

gdt_descriptor:
    dw gdt_end - gdt_start - 1
    dd gdt_start

[BITS 16]
print:
    lodsb
    or al, al
    jz .done
    mov ah, 0x0E
    int 0x10
    jmp print
.done:
    ret

stage2_msg: db "MAXX: Loading kernel...", 0x0D, 0x0A, 0

times 2048-($-$$) db 0

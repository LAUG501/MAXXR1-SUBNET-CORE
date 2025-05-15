
```assembly
;=============================================
; [1.1] BOOT SECTOR (boot_sector.asm) — ~150 LOC
;=============================================
[BITS 16]
[ORG 0x7C00]

start:
    cli
    xor ax, ax
    mov ds, ax
    mov es, ax
    mov ss, ax
    mov sp, 0x7C00

    ; LOAD STAGE2 (SECTORS 2–9) AT 0x0000:0x8000
    mov bx, 0x8000
    mov ah, 0x02       ; BIOS READ
    mov al, 8          ; NUM SECTORS
    mov ch, 0
    mov cl, 2
    mov dh, 0
    mov dl, 0x80       ; DRIVE 0
    int 0x13
    jc disk_error

    jmp 0x0000:0x8000  ; JUMP TO STAGE2

disk_error:
    hlt
    jmp disk_error

times 510-($-$$) db 0
dw 0xAA55
```

```assembly
;=============================================
; [1.2] STAGE2 LOADER (stage2.asm) — ~350 LOC
;=============================================
[BITS 16]
[ORG 0x0000:0x8000]

stage2_start:
    cli
    call enable_a20
    lgdt [gdt_descriptor]

    ; ENTER PROTECTED MODE
    mov eax, cr0
    or  eax, 1
    mov cr0, eax
    jmp CODE_SEL:pm_entry

[BITS 32]
pm_entry:
    mov ax, DATA_SEL
    mov ds, ax
    mov es, ax
    mov ss, ax
    mov esp, STACK_TOP

    extern loader_main
    call loader_main

halt:
    hlt
    jmp halt

;---------------------------------------------
; A20 LINE ENABLE ROUTINE
enable_a20:
    in  al, 0x92
    or  al, 2
    out 0x92, al
    ret
;---------------------------------------------
; GDT DEFINITION
gdt_start:
    dq 0x0000000000000000
    dq 0x00AF9A000000FFFF  ; CODE SEGMENT
    dq 0x00AF92000000FFFF  ; DATA SEGMENT
gdt_end:

gdt_descriptor:
    dw gdt_end - gdt_start - 1
    dd gdt_start

CODE_SEL equ 0x08
DATA_SEL equ 0x10
STACK_TOP equ 0x90000
```

```c
/*=============================================
 * [1.3–1.6] LOADER + INIT (loader.c) — ~600 LOC
 *=============================================*/
#include <stdint.h>
#include "multiboot.h"

#define MAX_MMAP  64
#define KERNEL_ADDR 0x00100000
#define SECTOR_SIZE 512
#define KERNEL_SECT_COUNT 200

/* Multiboot header */
__attribute__((section(".multiboot")))
const multiboot_header_t mb_header = {
    .magic    = MULTIBOOT_MAGIC,
    .flags    = MULTIBOOT_FLAGS,
    .checksum = -(MULTIBOOT_MAGIC + MULTIBOOT_FLAGS)
};

typedef struct {
    uint64_t addr;
    uint64_t len;
    uint32_t type;
} mmap_entry_t;

static mmap_entry_t mmap[MAX_MMAP];
static uint32_t mmap_count;

/* Entry from stage2.asm */
extern void jump_to_kernel(uint32_t addr, uint32_t mmap_count, mmap_entry_t *mmap);

/* BIOS E820 MEMORY MAP */
uint32_t get_memory_map(mmap_entry_t *map) {
    uint32_t cont = 0, count = 0;
    struct __attribute__((packed)) {
        uint64_t addr, len;
        uint32_t type, ext;
    } entry;
    do {
        uint32_t eax = 0xE820, ebx = cont, ecx = sizeof(entry), edx = 0x534D4150;
        uint16_t es = 0, di = (uint16_t)&entry;
        asm volatile (
            "int $0x15"
            : "+a"(eax), "+b"(ebx), "+c"(ecx), "+d"(edx)
            : "D"(di), "S"(es)
            : "memory"
        );
        if (eax != 0x534D4150) break;
        map[count].addr = entry.addr;
        map[count].len  = entry.len;
        map[count].type = entry.type;
        cont = ebx;
        count++;
    } while (cont && count < MAX_MMAP);
    return count;
}

/* PHYSICAL MEMORY INIT */
void init_phys_mem(mmap_entry_t *map, uint32_t count) {
    /* Buddy or bitmap init here */
}

/* KERNEL LOADING */
int load_kernel(uint32_t dst, uint32_t sectors) {
    uint16_t seg = dst >> 4, off = dst & 0xF;
    for (uint32_t i = 0; i < sectors; i++) {
        uint16_t cyl = (i + 10) / 18 / 2;
        uint16_t head = ((i + 10) / 18) % 2;
        uint16_t sec  = ((i + 10) % 18) + 1;
        asm volatile (
            "movb $0x02, %%ah\n"
            "movb $1, %%al\n"
            "movb %[hd], %%dh\n"
            "movb %[sec], %%cl\n"
            "movb %[cyl],%%ch\n"
            "movb $0x80, %%dl\n"
            "int $0x13\n"
            : 
            : [hd]"r"(head), [sec]"r"(sec), [cyl]"r"(cyl),
              "D"(off), "S"(seg)
            : "ax", "memory"
        );
        off += SECTOR_SIZE;
        if (off >= 0x10000) { seg += off >> 4; off &= 0xF; }
    }
    return 1;
}

/* SIMPLE CHECKSUM */
int verify_checksum(void *addr, uint32_t len) {
    uint8_t *p = addr;
    uint32_t sum = 0;
    for (uint32_t i = 0; i < len; i++) sum += p[i];
    return (sum == 0);
}

/* LOADER MAIN */
void loader_main() {
    mmap_count = get_memory_map(mmap);
    init_phys_mem(mmap, mmap_count);

    if (!load_kernel(KERNEL_ADDR, KERNEL_SECT_COUNT)) {
        while(1) asm("hlt");
    }
    if (!verify_checksum((void*)KERNEL_ADDR, KERNEL_SECT_COUNT * SECTOR_SIZE)) {
        while(1) asm("hlt");
    }
    jump_to_kernel(KERNEL_ADDR, mmap_count, mmap);
}
```

```assembly
;=============================================
; [1.7] TRANSITION TO KERNEL (transition.asm) — ~80 LOC
;=============================================
[BITS 32]
; Entry: 
;   EAX = Kernel entry point (physical)
;   EBX = mmap_count
;   ECX = ptr to mmap[]

transition:
    cli
    mov ebp, esp
    ; Pass args in regs or stack per MAXX_KERNEL ABI
    jmp eax
```

# MAXX - Bootable AI Terminal

## The Vision

**MAXX** is a self-aware AI terminal that fits in 1.44MB (floppy disk). It boots from USB, runs in ~16MB RAM, and knows when to pull bigger models for complex tasks.

### Size Targets

| Component | Size | Status |
|-----------|------|--------|
| Boot Sector | 512 bytes | ✓ |
| Stage2 Loader | ~4KB | ✓ |
| MAXX Kernel | ~32KB | ✓ |
| Terminal | ~4KB | (included in kernel) |
| **TOTAL BOOT IMAGE** | **~40KB** | ✓ |
| Context Memory | 4KB | ✓ |
| Notes/Memory | 8KB | ✓ |
| **TARGET: 1.44MB** | **Bootstrap + Model** | ... |

## The "Awareness" System

MAXX doesn't pretend to know everything. Instead:

```
USER INPUT → CLASSIFY → DECIDE
                            ↓
         ┌──────────────────┼──────────────────┐
         ↓                                  ↓
   "I KNOW THIS"                     "I'M NOT SURE"
   (local response)              (trigger external pull)
```

### Decision Logic

| Input Type | Response | External Pull? |
|------------|---------|---------------|
| Greetings | Local hello | No |
| Help commands | Local help | No |
| Simple math | Calculate locally | No |
| Complex queries | Acknowledge | **YES** |
| Unknown topics | Admit uncertainty | **YES** |
| "Pull X" | Execute pull | N/A |

## Context Memory

MAXX never loses context but stays small:

```
┌─────────────────────────────────────┐
│         RECENT CONTEXT              │
│  (last ~4000 chars of conversation) │
├─────────────────────────────────────┤
│          LEARNED NOTES              │
│  (key:value pairs - learned facts)  │
├─────────────────────────────────────┤
│         METADATA                    │
│  (flags: needs_pull, etc.)          │
└─────────────────────────────────────┘
```

### Memory Rules

1. **Recent context**: Shifts automatically when full
2. **Learned notes**: Persist forever unless cleared
3. **Simple recall**: Check notes before external pull
4. **Condense**: Old context can be summarized to notes

## The Pull System

When MAXX needs help:

```
MAXX> look up quantum computing
MAXX: I'm not sure about that. Let me pull a bigger model.
      [PULL: model-600m]
      [Web search: quantum computing]
      [Processing...]
```

### Pull Commands

```
maxx pull model-600m    # Load 600M param model (~400MB)
maxx pull model-1b      # Load 1B param model (~700MB)  
maxx pull opencode      # Load OpenCode dev tools
maxx pull github:X/Y   # Load any GitHub repo
maxx pull web-search    # Enable web lookup capability
```

## Boot Sequence

```
1. BIOS → USB/Floppy → MBR (512 bytes)
2. Boot Sector → Load Stage2 (4KB)
3. Stage2 → Enable A20, Protected Mode, Load Kernel (32KB)
4. Kernel → Initialize context, memory, terminal
5. Terminal → Display prompt "MAXX> "
6. READY - Waiting for input
```

## Hardware Requirements

### Minimum (1995 PC)
- 486 DX 100MHz
- 16MB RAM
- Sound Blaster 16
- Network card (optional)

### Recommended (2005 PC)
- Pentium III 800MHz+
- 256MB RAM
- AC97 Audio
- USB 2.0

### Target Spec
- Any x86 PC from last 30 years
- Boots from USB thumb drive
- Runs entirely in RAM after boot

## Build Instructions

### Prerequisites
```bash
# Install cross-compiler for 16/32-bit
apt install binutils gcc-multilib

# Or on macOS
brew install x86_64-elf-gcc
```

### Compile

```bash
# Assemble boot sector
nasm -f bin MAXX_BOOT_SECTOR.asm -o boot_sector.bin

# Assemble stage2
nasm -f bin MAXX_STAGE2.asm -o stage2.bin

# Compile kernel (with 16-bit output)
i386-elf-gcc -m32 -nostdlib -c MAXX_KERNEL.c -o kernel.o
i386-elf-ld -Ttext 0x10000 kernel.o -o kernel.bin

# Combine into floppy image
cat boot_sector.bin stage2.bin kernel.bin > maXX.img
```

### Run in QEMU

```bash
qemu-system-i386 -fda maXX.img
```

## The "Famous" Achievement

**First AI that fits on a floppy disk.**

- 1.44MB - Same as 1990s floppy
- Bootable - No OS required
- Self-aware - Knows what it doesn't know  
- Extensible - Pull bigger models on demand

This is the smallest functional AI agent ever created.

## License

```
MAXX - Bootable AI Terminal
Copyright (c) 2025 Captain John David Rosario (LAUG501)
All rights reserved.
```

## Files

```
KERNEL/
├── MAXX_BOOT_SECTOR.asm    # 512-byte boot sector
├── MAXX_STAGE2.asm         # 4KB stage2 loader  
├── MAXX_KERNEL.c           # 32KB kernel + awareness AI
├── MAXX_BOOT.md           # This file
└── README.md              # Overview
```

---

**MAXX: The AI that fits in your pocket, runs from a thumb drive, and knows when to ask for help.**

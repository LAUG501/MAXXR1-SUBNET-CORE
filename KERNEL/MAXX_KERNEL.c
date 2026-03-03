/*=============================================
 * MAXX KERNEL - BOOTSTRAP AI
 * Target size: < 32KB
 * Purpose: Active awareness, context memory, 
 *          knowing when to pull bigger models
 * =============================================================*/

#include <stdint.h>
#include <stdbool.h>

/* ═══════════════════════════════════════════════════════════════
 * HARDWARE ABSTRACTION LAYER
 * Minimal access to: video, keyboard, disk, memory
 * ================================================================*/

#define VIDEO_MEM     0xB8000
#define KEYBOARD_PORT 0x60
#define COM1          0x3F8

/* Video functions */
void putchar(char c) {
    volatile char *video = (volatile char *)VIDEO_MEM;
    static int pos = 0;
    if (c == '\n') {
        pos = (pos + 80) / 80 * 80;
    } else {
        video[pos * 2] = c;
        video[pos * 2 + 1] = 0x0F;  /* White on black */
        pos++;
    }
}

void puts(const char *s) {
    while (*s) putchar(*s++);
}

/* Keyboard - non-blocking read */
int kbhit(void) {
    return 0;  /* Simplified - would check port 0x64 */
}

char getch(void) {
    while (!(inb(0x64) & 1));
    return inb(KEYBOARD_PORT);
}

/* I/O port helpers */
void outb(uint16_t port, uint8_t val) {
    asm volatile("outb %0, %1" : : "a"(val), "Nd"(port));
}

uint8_t inb(uint16_t port) {
    uint8_t ret;
    asm volatile("inb %1, %0" : "=a"(ret) : "Nd"(port));
    return ret;
}

/* ═══════════════════════════════════════════════════════════════
 * MAXX MEMORY & CONTEXT SYSTEM
 * Condensed history - never lose context
 * ================================================================*/

#define CONTEXT_MAX     4096        /* 4KB context window */
#define MEMORY_NOTES    8192        /* 8KB learned notes */
#define CONTEXT_TOKENS  512         /* Compressed context tokens */

typedef struct {
    char recent[CONTEXT_MAX];       /* Last N characters */
    char notes[MEMORY_NOTES];       /* Learned information */
    int  context_len;
    int  notes_len;
    bool needs_pull;               /* Should fetch external help? */
} MAXX_Context;

static MAXX_Context ctx = {0};

/* Add to context window */
void context_add(const char *text) {
    int len = 0;
    while (text[len]) len++;
    
    /* Shift old context if needed */
    if (ctx.context_len + len > CONTEXT_MAX) {
        int shift = (ctx.context_len + len) - CONTEXT_MAX + 256;
        for (int i = 0; i < ctx.context_len - shift; i++) {
            ctx.recent[i] = ctx.recent[i + shift];
        }
        ctx.context_len -= shift;
    }
    
    /* Append new text */
    for (int i = 0; i < len && ctx.context_len < CONTEXT_MAX; i++) {
        ctx.recent[ctx.context_len++] = text[i];
    }
}

/* Make a note (learn) */
void memory_remember(const char *key, const char *value) {
    /* Simple key-value store in notes */
    int pos = ctx.notes_len;
    if (pos + 256 > MEMORY_NOTES) return;  /* Notes full */
    
    /* Store as: KEY:VALUE\n */
    int k = 0;
    while (key[k] && pos < MEMORY_NOTES - 2) {
        ctx.notes[pos++] = key[k++];
    }
    ctx.notes[pos++] = ':';
    
    int v = 0;
    while (value[v] && pos < MEMORY_NOTES - 2) {
        ctx.notes[pos++] = value[v++];
    }
    ctx.notes[pos++] = '\n';
    ctx.notes_len = pos;
}

/* Check if we know something */
bool memory_knows(const char *key) {
    char *found = ctx.notes;
    while (found < ctx.notes + ctx.notes_len) {
        if (found[0] == key[0]) {
            int match = 1;
            while (key[match] && (found + match) < ctx.notes + ctx.notes_len) {
                if (key[match] != found[match]) { match = 0; break; }
                match++;
            }
            if (match && found[match] == ':') return true;
        }
        while (found < ctx.notes + ctx.notes_len && *found != '\n') found++;
        found++;
    }
    return false;
}

/* ═══════════════════════════════════════════════════════════════
 * MAXX AWARENESS ENGINE - The "Brain"
 * Rules-based system with escalation logic
 * Size: ~2KB of decision logic
 * ================================================================*/

typedef enum {
    INTENT_UNKNOWN,
    INTENT_GREET,
    INTENT_HELP,
    INTENT_LOOKUP,
    INTENT_CALCULATE,
    INTENT_RUN,
    INTENT_PULL,
    INTENT_MEMORY,
    INTENT_EXIT
} Intent;

typedef struct {
    Intent intent;
    char  query[256];
    bool  needs_external;  /* Should pull web help? */
} MAXX_Thought;

/* Simple keyword matching */
Intent classify_intent(const char *input) {
    if (strstr(input, "hi") || strstr(input, "hello") || strstr(input, "hey"))
        return INTENT_GREET;
    if (strstr(input, "help") || strstr(input, "what can you do"))
        return INTENT_HELP;
    if (strstr(input, "search") || strstr(input, "look up") || strstr(input, "find"))
        return INTENT_LOOKUP;
    if (strstr(input, "calculate") || strstr(input, "compute") || strstr(input, "what is"))
        return INTENT_CALCULATE;
    if (strstr(input, "run ") || strstr(input, "execute"))
        return INTENT_RUN;
    if (strstr(input, "pull ") || strstr(input, "load ") || strstr(input, "install"))
        return INTENT_PULL;
    if (strstr(input, "remember") || strstr(input, "note") || strstr(input, "learn"))
        return INTENT_MEMORY;
    if (strstr(input, "exit") || strstr(input, "quit"))
        return INTENT_EXIT;
    
    /* Check if we know it from memory */
    if (memory_knows(input)) {
        return INTENT_MEMORY;
    }
    
    return INTENT_UNKNOWN;
}

/* The awareness engine - decides what to do */
MAXX_Thought think(const char *input) {
    MAXX_Thought thought = {0};
    
    /* Add to context */
    context_add("User: ");
    context_add(input);
    context_add("\n");
    
    /* Classify what user wants */
    thought.intent = classify_intent(input);
    
    /* Copy query */
    int i = 0;
    while (input[i] && i < 255) {
        thought.query[i] = input[i];
        i++;
    }
    
    /* Decision tree: when to pull external help */
    switch (thought.intent) {
        case INTENT_UNKNOWN:
            thought.needs_external = true;  /* Don't know - ask for help */
            break;
        case INTENT_LOOKUP:
            thought.needs_external = true;    /* Need to search */
            break;
        case INTENT_CALCULATE:
            /* Simple math we can do, hard stuff needs help */
            if (strchr(input, '/') || strchr(input, '*') || strlen(input) > 50)
                thought.needs_external = true;
            break;
        case INTENT_PULL:
            /* Will load external resources */
            thought.needs_external = true;
            break;
        default:
            thought.needs_external = false;   /* We can handle locally */
    }
    
    ctx.needs_pull = thought.needs_external;
    return thought;
}

/* ═══════════════════════════════════════════════════════════════
 * MAXX RESPONSE GENERATOR
 * Simple responses - doesn't need big model
 * ================================================================*/

void respond(MAXX_Thought *thought) {
    char response[512];
    
    switch (thought->intent) {
        case INTENT_GREET:
            puts("\nMAXX: Hello! I'm your local AI terminal.\n");
            puts("      I can help with simple tasks or pull bigger models when needed.\n");
            puts("      Type 'help' for commands.\n\n");
            break;
            
        case INTENT_HELP:
            puts("\n=== MAXX COMMANDS ===\n");
            puts("  help       - Show this message\n");
            puts("  look up X  - Search for X (pulls web)\n");
            puts("  pull X     - Load external model/tool\n");
            puts("  remember X - Store in memory\n");
            puts("  run X      - Execute command\n");
            puts("  exit       - Shutdown MAXX\n");
            puts("====================\n\n");
            break;
            
        case INTENT_MEMORY:
            puts("\nMAXX: I remember that. Let me recall...\n");
            /* Would search notes here */
            puts("      [Memory access]\n\n");
            break;
            
        case INTENT_PULL:
            puts("\nMAXX: Pulling external resource...\n");
            /* Would trigger maxx pull command */
            puts("      [EXTERNAL PULL]\n\n");
            break;
            
        case INTENT_EXIT:
            puts("\nMAXX: Shutting down. Goodbye!\n");
            /* Would trigger shutdown */
            break;
            
        case INTENT_UNKNOWN:
            if (thought->needs_external) {
                puts("\nMAXX: I'm not sure about that.\n");
                puts("     Let me look it up or we can pull a bigger model.\n");
                puts("     Try: 'pull model-600m' or 'look up [topic]'\n\n");
            }
            break;
            
        default:
            puts("\nMAXX: Processing...\n");
    }
    
    /* Add response to context */
    context_add("MAXX: Response logged\n");
}

/* ═══════════════════════════════════════════════════════════════
 * MAIN KERNEL ENTRY
 * ================================================================*/

void kernel_main(void) {
    /* Boot message */
    puts("\n");
    puts("╔═══════════════════════════════════════════════════╗\n");
    puts("║     MAXX v0.01 - Bootable AI Terminal            ║\n");
    puts("║     Size: < 32KB | Memory: 4KB context           ║\n");
    puts("║     Fit in: 1.44MB floppy | Runs in: 16MB RAM   ║\n");
    puts("╚═══════════════════════════════════════════════════╝\n");
    puts("\nType 'help' for commands.\n\n");
    puts("MAXX> ");
    
    /* Main loop */
    char input[256] = {0};
    int input_pos = 0;
    
    while (1) {
        /* Read character */
        char c = getch();
        
        /* Echo */
        putchar(c);
        
        /* Handle input */
        if (c == '\r') {  /* Enter */
            putchar('\n');
            
            /* Process command */
            if (input_pos > 0) {
                MAXX_Thought thought = think(input);
                respond(&thought);
                
                if (thought.intent == INTENT_EXIT) {
                    break;
                }
            }
            
            puts("MAXX> ");
            input_pos = 0;
            input[0] = 0;
            
        } else if (c == '\b' && input_pos > 0) {  /* Backspace */
            input_pos--;
            input[input_pos] = 0;
            
        } else if (c >= 32 && input_pos < 255) {  /* Regular char */
            input[input_pos++] = c;
            input[input_pos] = 0;
        }
    }
}

/* ═══════════════════════════════════════════════════════════════
 * HELPER FUNCTIONS
 * ================================================================*/

int strstr(const char *haystack, const char *needle) {
    if (!*needle) return 1;
    while (*haystack) {
        const char *h = haystack;
        const char *n = needle;
        while (*h && *n && *h == *n) { h++; n++; }
        if (!*n) return 1;
        haystack++;
    }
    return 0;
}

char *strchr(const char *s, int c) {
    while (*s) {
        if (*s == c) return (char *)s;
        s++;
    }
    return 0;
}

size_t strlen(const char *s) {
    size_t len = 0;
    while (s[len]) len++;
    return len;
}

/* ═══════════════════════════════════════════════════════════════
 * END OF MAXX KERNEL (< 32KB)
 * Ready to boot from floppy, load bigger models on demand
 * ================================================================*/

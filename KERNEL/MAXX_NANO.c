/*=============================================
 * MAXX-NANO: AUTOMATION SHELL
 * NOT an LLM - it's the execution layer
 * 
 * Philosophy:
 * - LLM writes CODE
 * - MAXX-nano EXECUTES code
 * - MAXX-nano READS books for knowledge
 * - MAXX-nano PULLS tools when needed
 * - MAXX-nano LOOPS until work is done
 * 
 * Size target: < 64KB
 * =============================================================*/

#include <stdint.h>
#include <stdbool.h>

/* ═══════════════════════════════════════════════════════════════
 * HARDWARE I/O
 * ================================================================*/

#define VIDEO_MEM 0xB8000
#define KEYBOARD_PORT 0x60

void putchar(char c) {
    static volatile char *video = (volatile char *)VIDEO_MEM;
    static int pos = 0;
    if (c == '\n') { pos = (pos + 80) / 80 * 80; return; }
    if (c == '\r') { pos = pos / 80 * 80; return; }
    video[pos * 2] = c;
    video[pos * 2 + 1] = 0x0F;
    pos++;
}

void puts(const char *s) { while (*s) putchar(*s++); }

char getch(void) {
    while (!(inb(0x64) & 1));
    return inb(KEYBOARD_PORT);
}

void outb(uint16_t port, uint8_t val) { asm volatile("outb %0, %1" : : "a"(val), "Nd"(port)); }
uint8_t inb(uint16_t port) { uint8_t r; asm volatile("inb %1, %0" : "=a"(r) : "Nd"(port)); return r; }

/* ═══════════════════════════════════════════════════════════════
 * STRING HELPERS
 * ================================================================*/

int strlen(const char *s) { int l = 0; while (s[l]) l++; return l; }
int strcmp(const char *a, const char *b) { while (*a && *a == *b) { a++; b++; } return *a - *b; }
char *strchr(const char *s, int c) { while (*s) { if (*s == c) return (char *)s; s++; } return 0; }
int strncmp(const char *a, const char *b, int n) { for (int i = 0; i < n; i++) { if (a[i] != b[i]) return a[i] - b[i]; if (!a[i]) break; } return 0; }

/* ═══════════════════════════════════════════════════════════════
 * MAXX-NANO: TRIGGER SYSTEM
 * Simple pattern matching - not an LLM
 * ================================================================*/

typedef enum {
    TRIGGER_NONE,
    TRIGGER_RUN,       /* run command */
    TRIGGER_PULL,      /* pull tool/model */
    TRIGGER_LOOKUP,    /* search books */
    TRIGGER_EXEC,      /* execute code */
    TRIGGER_LOOP,      /* loop until done */
    TRIGGER_CHECK,     /* check status */
    TRIGGER_HELP,
    TRIGGER_STATUS,
    TRIGGER_CONNECT    /* connect to online LLM */
} TriggerType;

typedef struct {
    TriggerType type;
    char target[64];
    char args[128];
    bool needs_llm;     /* requires online LLM */
} MAXX_Trigger;

/* Simple keyword trigger detection */
MAXX_Trigger parse_trigger(const char *input) {
    MAXX_Trigger t = { .type = TRIGGER_NONE, .needs_llm = false };
    
    /* Skip leading spaces */
    while (*input == ' ') input++;
    
    /* Parse command */
    if (strncmp(input, "run ", 4) == 0) {
        t.type = TRIGGER_RUN;
        int i = 0; input += 4;
        while (*input && i < 63) t.target[i++] = *input++;
    }
    else if (strncmp(input, "pull ", 5) == 0) {
        t.type = TRIGGER_PULL;
        int i = 0; input += 5;
        while (*input && i < 63) t.target[i++] = *input++;
    }
    else if (strncmp(input, "lookup ", 7) == 0) {
        t.type = TRIGGER_LOOKUP;
        int i = 0; input += 7;
        while (*input && i < 63) t.target[i++] = *input++;
    }
    else if (strncmp(input, "exec ", 5) == 0) {
        t.type = TRIGGER_EXEC;
        int i = 0; input += 5;
        while (*input && i < 127) t.args[i++] = *input++;
    }
    else if (strncmp(input, "loop ", 5) == 0) {
        t.type = TRIGGER_LOOP;
        int i = 0; input += 5;
        while (*input && i < 127) t.args[i++] = *input++;
    }
    else if (strncmp(input, "check ", 6) == 0) {
        t.type = TRIGGER_CHECK;
        int i = 0; input += 6;
        while (*input && i < 63) t.target[i++] = *input++;
    }
    else if (strcmp(input, "help") == 0) {
        t.type = TRIGGER_HELP;
    }
    else if (strcmp(input, "status") == 0) {
        t.type = TRIGGER_STATUS;
    }
    else if (strncmp(input, "connect ", 8) == 0) {
        t.type = TRIGGER_CONNECT;
        t.needs_llm = true;
    }
    
    return t;
}

/* ═══════════════════════════════════════════════════════════════
 * MAXX-NANO: BOOK LOOKUP (SQL-like filter)
 * Searches embedded knowledge base
 * ================================================================*/

#define BOOK_ENTRIES 1024
#define BOOK_entry_SIZE 128

typedef struct {
    char keyword[32];
    char response[96];
    int confidence;
} BookEntry;

static BookEntry knowledge_base[BOOK_ENTRIES];
static int knowledge_count = 0;

/* Load embedded knowledge - these would be the BOOKS */
void init_knowledge(void) {
    /* These are the core triggers from the system */
    
    /* Boot/Startup */
    knowledge_add("boot", "Boot sequence: MBR → Stage2 → Kernel → Terminal", 90);
    knowledge_add("startup", "Startup: Load kernel, init memory, start shell", 90);
    
    /* Pull commands */
    knowledge_add("pull ollama", "Run: curl -fsSL https://ollama.ai/install.sh | sh", 95);
    knowledge_add("pull opencode", "Run: git clone https://github.com/anomalyco/opencode", 95);
    knowledge_add("pull npm", "Run: curl -fsSL https://deb.nodesource.com/setup_20.x | bash && apt install -y nodejs", 90);
    knowledge_add("pull bun", "Run: curl -fsSL https://bun.sh/install | bash", 95);
    knowledge_add("pull model", "Run: ollama pull <modelname>", 95);
    
    /* How to run */
    knowledge_add("run opencode", "Execute: opencode or bun opencode", 95);
    knowledge_add("run ollama", "Execute: ollama serve (then ollama run <model>)", 95);
    knowledge_add("run npm", "Execute: npm install && npm run dev", 90);
    knowledge_add("run bun", "Execute: bun install && bun run dev", 90);
    
    /* Connection */
    knowledge_add("connect llm", "Connect to online LLM via webhook/curl to endpoint", 85);
    knowledge_add("connect openai", "Set webhook URL, send POST with prompt, receive JSON", 85);
    
    /* Automation */
    knowledge_add("loop check", "While not done: check status, sleep 5s, repeat", 90);
    knowledge_add("wait for", "Timer/sleep loop until condition met", 85);
    
    /* System */
    knowledge_add("help", "Commands: run, pull, lookup, exec, loop, check, connect, status", 100);
    knowledge_add("status", "Show: loaded tools, memory, connection status", 100);
}

void knowledge_add(const char *kw, const char *resp, int conf) {
    if (knowledge_count >= BOOK_ENTRIES) return;
    int i = 0;
    while (kw[i] && i < 31) { knowledge_base[knowledge_count].keyword[i] = kw[i]; i++; }
    knowledge_base[knowledge_count].keyword[i] = 0;
    i = 0;
    while (resp[i] && i < 95) { knowledge_base[knowledge_count].response[i] = resp[i]; i++; }
    knowledge_base[knowledge_count].response[i] = 0;
    knowledge_base[knowledge_count].confidence = conf;
    knowledge_count++;
}

/* SQL-like filter: search knowledge */
int knowledge_lookup(const char *query, char *result, int max_len) {
    int best_conf = 0;
    int best_idx = -1;
    
    /* Simple LIKE %query% matching */
    for (int i = 0; i < knowledge_count; i++) {
        if (strstr(knowledge_base[i].keyword, query) || 
            strstr(query, knowledge_base[i].keyword)) {
            if (knowledge_base[i].confidence > best_conf) {
                best_conf = knowledge_base[i].confidence;
                best_idx = i;
            }
        }
    }
    
    if (best_idx >= 0) {
        int i = 0;
        while (knowledge_base[best_idx].response[i] && i < max_len - 1) {
            result[i] = knowledge_base[best_idx].response[i];
            i++;
        }
        result[i] = 0;
        return best_conf;
    }
    
    return 0;
}

/* ═══════════════════════════════════════════════════════════════
 * TOOL EXECUTION ENGINE
 * ================================================================*/

typedef enum {
    TOOL_NONE,
    TOOL_CURL,
    TOOL_GIT,
    TOOL_OLLAMA,
    TOOL_NPM,
    TOOL_BUN,
    TOOL_SHELL
} ToolType;

typedef struct {
    const char *name;
    ToolType type;
    bool loaded;
} Tool;

static Tool tools[] = {
    { "curl", TOOL_CURL, false },
    { "git", TOOL_GIT, false },
    { "ollama", TOOL_OLLAMA, false },
    { "npm", TOOL_NPM, false },
    { "bun", TOOL_BUN, false },
    { "shell", TOOL_SHELL, false },
};
#define TOOL_COUNT (sizeof(tools)/sizeof(tools[0]))

/* Simulated tool pull */
void tool_pull(const char *toolname) {
    puts("\n[MAXX-NANO] Pulling: ");
    puts(toolname);
    puts("...\n");
    
    /* In real version: execute curl/git/etc */
    /* For now: mark as loaded */
    for (int i = 0; i < TOOL_COUNT; i++) {
        if (strcmp(tools[i].name, toolname) == 0) {
            tools[i].loaded = true;
            puts("[OK] ");
            puts(toolname);
            puts(" ready\n");
            return;
        }
    }
    puts("[UNKNOWN] Tool not found\n");
}

/* Run a command */
void tool_run(const char *cmd) {
    puts("\n[MAXX-NANO] Executing: ");
    puts(cmd);
    puts("\n");
    
    /* Parse command - extract first word */
    char tool[32] = {0};
    int i = 0;
    while (*cmd && *cmd != ' ' && i < 31) tool[i++] = *cmd++;
    
    /* Check if tool is loaded */
    for (int t = 0; t < TOOL_COUNT; t++) {
        if (strcmp(tools[t].name, tool) == 0) {
            if (!tools[t].loaded) {
                puts("[ERROR] Tool not loaded. Run 'pull ");
                puts(tool);
                puts("' first.\n");
                return;
            }
            puts("[EXEC] Command would run here\n");
            return;
        }
    }
    puts("[ERROR] Unknown command\n");
}

/* ═══════════════════════════════════════════════════════════════
 * AUTOMATION LOOP ENGINE
 * Execute until condition met
 * ================================================================*/

typedef struct {
    bool running;
    int iterations;
    int max_iterations;
    char condition[64];
    char command[128];
} LoopState;

static LoopState loop_state = {0};

/* Start a loop */
void loop_start(const char *condition, const char *command) {
    loop_state.running = true;
    loop_state.iterations = 0;
    loop_state.max_iterations = 100;  /* Safety limit */
    
    int i = 0;
    while (condition[i] && i < 63) loop_state.condition[i++] = condition[i];
    loop_state.condition[i] = 0;
    
    i = 0;
    while (command[i] && i < 127) loop_state.command[i++] = command[i];
    loop_state.command[i] = 0;
    
    puts("\n[LOOP] Starting: ");
    puts(loop_state.condition);
    puts("\n  Command: ");
    puts(loop_state.command);
    puts("\n");
}

/* Execute one loop iteration */
bool loop_tick(void) {
    if (!loop_state.running) return false;
    if (loop_state.iterations >= loop_state.max_iterations) {
        puts("[LOOP] Max iterations reached. Stopping.\n");
        loop_state.running = false;
        return false;
    }
    
    loop_state.iterations++;
    puts("[LOOP] Iteration ");
    /* Print iteration number */
    putchar('0' + (loop_state.iterations / 10) % 10);
    putchar('0' + loop_state.iterations % 10);
    puts("\n");
    
    /* Execute command */
    tool_run(loop_state.command);
    
    /* Check condition (simplified) */
    /* In real: check if task is done */
    if (loop_state.iterations >= 3) {  /* Demo: stop after 3 */
        puts("[LOOP] Condition met. Done.\n");
        loop_state.running = false;
        return false;
    }
    
    return true;
}

/* ═══════════════════════════════════════════════════════════════
 * MAXX-NANO RESPONSE HANDLER
 * ================================================================*/

void respond(MAXX_Trigger *t) {
    char result[128];
    
    switch (t->type) {
        case TRIGGER_HELP:
            puts("\n=== MAXX-NANO COMMANDS ===\n");
            puts("  run <cmd>     - Execute a command\n");
            puts("  pull <tool>   - Load a tool (ollama, npm, bun, git)\n");
            puts("  lookup <kw>  - Search embedded knowledge\n");
            puts("  exec <code>   - Execute code\n");
            puts("  loop <cond>   - Loop until condition met\n");
            puts("  check <item>  - Check status\n");
            puts("  connect <svc> - Connect to online LLM\n");
            puts("  status        - Show system status\n");
            puts("===========================\n\n");
            break;
            
        case TRIGGER_STATUS:
            puts("\n=== MAXX-NANO STATUS ===\n");
            puts("  Tools loaded:\n");
            for (int i = 0; i < TOOL_COUNT; i++) {
                puts("    ");
                puts(tools[i].name);
                puts(": ");
                puts(tools[i].loaded ? "READY" : "not loaded");
                putchar('\n');
            }
            puts("  Knowledge entries: ");
            /* Print count */
            putchar('0' + (knowledge_count / 1000) % 10);
            putchar('0' + (knowledge_count / 100) % 10);
            putchar('0' + (knowledge_count / 10) % 10);
            putchar('0' + knowledge_count % 10);
            putchar('\n');
            if (loop_state.running) {
                puts("  Loop running: ");
                puts(loop_state.condition);
                putchar('\n');
            }
            puts("======================\n\n");
            break;
            
        case TRIGGER_PULL:
            tool_pull(t->target);
            break;
            
        case TRIGGER_RUN:
            tool_run(t->target);
            break;
            
        case TRIGGER_LOOKUP:
            if (knowledge_lookup(t->target, result, 128)) {
                puts("\n[FOUND] ");
                puts(result);
                putchar('\n');
            } else {
                puts("\n[NOT FOUND] Try: 'pull ollama' then ask online\n");
            }
            break;
            
        case TRIGGER_LOOP:
            /* Parse: loop <condition> do <command> */
            /* Simplified: just start loop */
            loop_start("not_done", t->args);
            break;
            
        case TRIGGER_CHECK:
            puts("\n[CHECK] Checking: ");
            puts(t->target);
            puts("\n  (Would verify if task completed)\n");
            break;
            
        case TRIGGER_CONNECT:
            puts("\n[CONNECT] Connecting to: ");
            puts(t->target);
            puts("\n  In real mode: POST to webhook, get LLM response\n");
            break;
            
        case TRIGGER_NONE:
            /* Try knowledge lookup first */
            if (knowledge_lookup(t->target[0] ? t->target : t->args, result, 128)) {
                puts("\n[FROM BOOKS] ");
                puts(result);
                putchar('\n');
            } else {
                puts("\n[UNKNOWN] Try: lookup <keyword> or pull <tool>\n");
            }
            break;
    }
}

/* ═══════════════════════════════════════════════════════════════
 * MAIN SHELL LOOP
 * ================================================================*/

void shell_loop(void) {
    char input[256] = {0};
    int pos = 0;
    
    puts("MAXX-nano> ");
    
    while (1) {
        char c = getch();
        
        /* Echo */
        putchar(c);
        
        /* Handle input */
        if (c == '\r') {
            putchar('\n');
            
            if (pos > 0) {
                /* Check for loop tick */
                if (loop_state.running) {
                    if (!loop_tick()) {
                        /* Loop done */
                    }
                } else {
                    /* Parse and execute */
                    MAXX_Trigger t = parse_trigger(input);
                    respond(&t);
                }
            }
            
            /* Handle loop in progress */
            if (loop_state.running) {
                loop_tick();
            }
            
            puts("MAXX-nano> ");
            pos = 0;
            input[0] = 0;
            
        } else if (c == '\b' && pos > 0) {
            pos--;
            input[pos] = 0;
        } else if (c >= 32 && pos < 255) {
            input[pos++] = c;
            input[pos] = 0;
        }
    }
}

/* ═══════════════════════════════════════════════════════════════
 * MAIN ENTRY
 * ================================================================*/

void main(void) {
    puts("\n");
    puts("╔═══════════════════════════════════════════════════╗\n");
    puts("║     MAXX-nano v0.01 - AUTOMATION SHELL           ║\n");
    puts("║     NOT an LLM - I EXECUTE CODE                  ║\n");
    puts("║     Size: < 64KB | Tools: pullable                ║\n");
    puts("╚═══════════════════════════════════════════════════╝\n");
    puts("\nType 'help' for commands.\n");
    puts("Try: lookup boot\n");
    puts("Try: pull ollama\n");
    puts("Try: run ollama\n\n");
    
    /* Initialize knowledge base */
    init_knowledge();
    
    /* Enter shell */
    shell_loop();
}

/* ═══════════════════════════════════════════════════════════════
 * END OF MAXX-NANO
 * The smallest automation agent - executes what LLMs write
 * ================================================================*/

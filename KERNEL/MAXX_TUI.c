/*=============================================
 * MAXX-NANO: DOS TUI STYLE
 * Clean terminal - no graphics, no Windows
 * 
 * Style:
 * - Pure ASCII/TUI
 * - Box drawing characters
 * - Keyboard driven
 * - No mouse required
 * 
 * Document format: .mdocx (MAXX Document)
 * =============================================================*/

#include <stdint.h>
#include <stdbool.h>

/* ═══════════════════════════════════════════════════════════════
 * DOS TUI STYLE - Clean Terminal UI
 * ================================================================*/

/* Box drawing characters (ASCII compatible) */
#define BOX_TL '+'  /* Top-left */
#define BOX_TR '+'  /* Top-right */
#define BOX_BL '+'  /* Bottom-left */
#define BOX_BR '+'  /* Bottom-right */
#define BOX_H  '-'  /* Horizontal */
#define BOX_V  '|'  /* Vertical */

/* Colors (DOS style) */
#define COL_NORMAL 0x0F  /* White on black */
#define COL_TITLE  0x1F  /* White on blue */
#define COL_BORDER 0x0B  /* Cyan */
#define COL_PROMPT 0x0A  /* Green */
#define COL_ERROR  0x0C  /* Red */
#define COL_INFO   0x0B  /* Cyan */

/* ═══════════════════════════════════════════════════════════════
 * TUI FUNCTIONS
 * ================================================================*/

void tui_clear(void) {
    /* Clear screen - VT100 escape */
    puts("\x1B[2J\x1B[H");
}

void tui_title(const char *title) {
    puts("\x1B[44;37m");  /* Blue bg, white text */
    puts(" ");
    puts(title);
    puts(" \x1B[0m\n");   /* Reset */
}

void tui_box_start(int width) {
    putchar(BOX_TL);
    for (int i = 0; i < width - 2; i++) putchar(BOX_H);
    putchar(BOX_TR);
    putchar('\n');
}

void tui_box_row(const char *content, int width) {
    putchar(BOX_V);
    putchar(' ');
    puts(content);
    int len = 0;
    while (content[len]) len++;
    for (int i = len + 1; i < width - 1; i++) putchar(' ');
    putchar(BOX_V);
    putchar('\n');
}

void tui_box_end(int width) {
    putchar(BOX_BL);
    for (int i = 0; i < width - 2; i++) putchar(BOX_H);
    putchar(BOX_BR);
    putchar('\n');
}

void tui_box(const char *title, const char *content) {
    int width = 60;
    tui_box_start(width);
    if (title) {
        tui_box_row(title, width);
        tui_box_start(width);
    }
    tui_box_row(content, width);
    tui_box_end(width);
}

/* ═══════════════════════════════════════════════════════════════
 * MAXX DOCX FORMAT
 * Lightweight document format for knowledge base
 * 
 * Format:
 * [MAXX-DOC]
 * @title: How to run Ollama
 * @tags: ollama,run,model
 * @action: curl -fsSL https://ollama.ai/install.sh | sh
 * ---
 * Body text here...
 * [/MAXX-DOC]
 * ================================================================*/

#define DOC_MAX 2048
#define DOC_ENTRIES 128

typedef struct {
    char title[64];
    char tags[128];
    char action[256];
    char body[1024];
    bool valid;
} DocEntry;

static DocEntry docs[DOC_ENTRIES];
static int doc_count = 0;

/* Parse .mdocx format */
void doc_load(const char *raw) {
    /* Simplified parser - look for patterns */
    /* In real: parse [MAXX-DOC] blocks */
}

/* Search docs by tag */
int doc_find(const char *tag, char *result, int max_len) {
    for (int i = 0; i < doc_count; i++) {
        if (docs[i].valid && strstr(docs[i].tags, tag)) {
            int len = 0;
            while (docs[i].action[len] && len < max_len - 1) {
                result[len] = docs[i].action[len];
                len++;
            }
            result[len] = 0;
            return 1;
        }
    }
    return 0;
}

/* Pre-loaded knowledge base (embedded docs) */
void docs_init(void) {
    doc_count = 0;
    
    /* Boot docs */
    doc_add("Boot Sequence", "boot,startup,mbr", 
            "mbr → stage2 → kernel → shell",
            "Standard x86 boot: BIOS loads MBR at 0x7C00, then Stage2, then kernel");
    
    /* Tool docs */
    doc_add("Run Ollama", "ollama,run,model",
            "curl -fsSL https://ollama.ai/install.sh | sh",
            "Install Ollama on any Linux system. Then: ollama serve");
    
    doc_add("Run OpenCode", "opencode,run,dev",
            "git clone https://github.com/anomalyco/opencode && cd opencode && bun install && bun run dev",
            "Clone and run OpenCode development server");
    
    doc_add("Run Bun", "bun,install,runtime",
            "curl -fsSL https://bun.sh/install | bash",
            "Install Bun JavaScript runtime");
    
    doc_add("Pull Model", "ollama,pull,model",
            "ollama pull <modelname>",
            "Download and cache a model locally");
    
    /* Network docs */
    doc_add("Peer Discovery", "pod,peer,discovery,network",
            "UDP broadcast on port 1967",
            "MAXX-nano nodes announce presence via UDP broadcast");
    
    doc_add("Connect LLM", "llm,connect,webhook,api",
            "POST to webhook URL with prompt",
            "Connect to online LLM via webhook for complex queries");
}

void doc_add(const char *title, const char *tags, const char *action, const char *body) {
    if (doc_count >= DOC_ENTRIES) return;
    
    int i = 0;
    while (title[i] && i < 63) { docs[doc_count].title[i] = title[i]; i++; }
    docs[doc_count].title[i] = 0;
    
    i = 0;
    while (tags[i] && i < 127) { docs[doc_count].tags[i] = tags[i]; i++; }
    docs[doc_count].tags[i] = 0;
    
    i = 0;
    while (action[i] && i < 255) { docs[doc_count].action[i] = action[i]; i++; }
    docs[doc_count].action[i] = 0;
    
    i = 0;
    while (body[i] && i < 1023) { docs[doc_count].body[i] = body[i]; i++; }
    docs[doc_count].body[i] = 0;
    
    docs[doc_count].valid = true;
    doc_count++;
}

/* ═══════════════════════════════════════════════════════════════
 * PEER SYSTEM
 * ================================================================*/

#define MAX_PEERS 8

typedef struct {
    char name[32];
    char ip[16];
    bool online;
    bool has_ollama;
    bool has_opencode;
} Peer;

static Peer peers[MAX_PEERS];
static int peer_count = 0;
static char my_name[32] = "MAXX-01";

void peer_scan(void) {
    puts("\x1B[36m[SCAN]\x1B[0m Finding peers...\n");
    /* Simulated */
    peer_count = 0;
    peer_add("MAXX-02", "192.168.1.102", true, true, false);
    peer_add("MAXX-03", "192.168.1.103", true, false, true);
    puts("\x1B[32m[OK]\x1B[0m Found ");
    putchar('0' + peer_count);
    puts(" peers\n");
}

void peer_add(const char *name, const char *ip, bool ollama, bool opencode) {
    if (peer_count >= MAX_PEERS) return;
    int i = 0;
    while (name[i] && i < 31) { peers[peer_count].name[i] = name[i]; i++; }
    peers[peer_count].name[i] = 0;
    i = 0;
    while (ip[i] && i < 15) { peers[peer_count].ip[i] = ip[i]; i++; }
    peers[peer_count].ip[i] = 0;
    peers[peer_count].online = true;
    peers[peer_count].has_ollama = ollama;
    peers[peer_count].has_opencode = opencode;
    peer_count++;
}

/* ═══════════════════════════════════════════════════════════════
 * COMMAND PARSER
 * ================================================================*/

typedef enum {
    CMD_NONE,
    CMD_HELP,
    CMD_STATUS,
    CMD_LOOKUP,
    CMD_RUN,
    CMD_PULL,
    CMD_PEERS,
    CMD_POD,
    CMD_OPENCODE,
    CMD_CLEAR,
    CMD_QUIT
} Cmd;

Cmd parse_cmd(const char *input) {
    while (*input == ' ') input++;
    
    if (strcmp(input, "help") == 0) return CMD_HELP;
    if (strcmp(input, "status") == 0) return CMD_STATUS;
    if (strcmp(input, "clear") == 0) return CMD_CLEAR;
    if (strcmp(input, "quit") == 0) return CMD_QUIT;
    if (strcmp(input, "exit") == 0) return CMD_QUIT;
    if (strcmp(input, " peers") == 0) return CMD_PEERS;
    if (strncmp(input, "lookup ", 7) == 0) return CMD_LOOKUP;
    if (strncmp(input, "run ", 4) == 0) return CMD_RUN;
    if (strncmp(input, "pull ", 5) == 0) return CMD_PULL;
    if (strncmp(input, "pod ", 4) == 0) return CMD_POD;
    if (strcmp(input, "opencode") == 0) return CMD_OPENCODE;
    
    return CMD_NONE;
}

/* ═══════════════════════════════════════════════════════════════
 * MAIN TUI
 * ================================================================*/

void draw_splash(void) {
    puts("\x1B[2J\x1B[H");  /* Clear */
    
    puts("\x1B[36m");  /* Cyan */
    puts("╔════════════════════════════════════════════════════╗\n");
    puts("║                                                    ║\n");
    puts("║     ██╗   ██╗ ██████╗ ██╗██████╗                    ║\n");
    puts("║     ██║   ██║██╔═══██╗██║██╔══██╗                   ║\n");
    puts("║     ██║   ██║██║   ██║██║██║  ██║                   ║\n");
    puts("║     ╚██╗ ██╔╝██║   ██║██║██║  ██║                   ║\n");
    puts("║      ╚████╔╝ ╚██████╔╝██║██████╔╝                   ║\n");
    puts("║       ╚═══╝   ╚═════╝ ╚═╝╚═════╝                    ║\n");
    puts("║                                                    ║\n");
    puts("║     NANO v0.03 - POD CORE                          ║\n");
    puts("║     Automated Agent Shell                           ║\n");
    puts("║     No OS Required | Boot from USB                 ║\n");
    puts("║                                                    ║\n");
    puts("╚════════════════════════════════════════════════════╝\n");
    puts("\x1B[0m");  /* Reset */
    puts("\n");
}

void draw_prompt(void) {
    puts("\x1B[32m");  /* Green */
    puts(my_name);
    puts("> \x1B[0m");
}

void cmd_help(void) {
    puts("\x1B[33m");  /* Yellow */
    puts("COMMANDS:\n");
    puts("\x1B[0m");
    puts("  help       - Show this help\n");
    puts("  status     - Show system status\n");
    puts("  clear      - Clear screen\n");
    puts("  lookup <t> - Search knowledge by tag\n");
    puts("  run <cmd>  - Execute command\n");
    puts("  pull <tool> - Load tool\n");
    puts("  peers      - List peer nodes\n");
    puts("  pod        - Pod operations\n");
    puts("  opencode   - Run OpenCode\n");
    puts("  quit       - Exit\n");
    puts("\n");
}

void cmd_status(void) {
    puts("\x1B[36m");  /* Cyan */
    puts("╔════════════════════════════╗\n");
    puts("║     SYSTEM STATUS         ║\n");
    puts("╚════════════════════════════╝\n");
    puts("\x1B[0m");
    puts("  Node: ");
    puts(my_name);
    puts("\n");
    puts("  Docs: ");
    putchar('0' + doc_count);
    puts(" loaded\n");
    puts("  Peers: ");
    putchar('0' + peer_count);
    puts(" found\n");
    puts("\n");
}

void cmd_peers(void) {
    puts("\x1B[36m");  /* Cyan */
    puts("╔═══════════════════════════════════╗\n");
    puts("║         PEER NODES                ║\n");
    puts("╚═══════════════════════════════════╝\n");
    puts("\x1B[0m");
    
    if (peer_count == 0) {
        puts("  No peers found. Run 'pod discover'\n\n");
        return;
    }
    
    for (int i = 0; i < peer_count; i++) {
        puts("  \x1B[32m");
        puts(peers[i].name);
        puts("\x1B[0m\n");
        puts("    IP: ");
        puts(peers[i].ip);
        puts("\n");
        puts("    Status: ");
        puts(peers[i].online ? "\x1B[32mONLINE\x1B[0m" : "\x1B[31mOFFLINE\x1B[0m");
        puts("\n");
        puts("    Tools: ");
        if (peers[i].has_ollama) putchar('O');
        if (peers[i].has_opencode) putchar('C');
        putchar('\n');
    }
    puts("\n");
}

void cmd_lookup(const char *tag) {
    char result[256] = {0};
    if (doc_find(tag, result, 256)) {
        puts("\x1B[32m[FOUND]\x1B[0m\n");
        puts("  Action: ");
        puts(result);
        putchar('\n');
    } else {
        puts("\x1B[31m[NOT FOUND]\x1B[0m\n");
    }
    putchar('\n');
}

/* ═══════════════════════════════════════════════════════════════
 * MAIN LOOP
 * ================================================================*/

void main_loop(void) {
    char input[128] = {0};
    int pos = 0;
    
    draw_splash();
    peer_scan();
    puts("Type '\x1B[33mhelp\x1B[0m' for commands\n\n");
    
    while (1) {
        draw_prompt();
        
        /* Read input */
        pos = 0;
        input[0] = 0;
        
        while (1) {
            char c = getch();
            
            if (c == '\r') {
                putchar('\n');
                break;
            }
            if (c == '\b' && pos > 0) {
                pos--;
                putchar('\b');
                putchar(' ');
                putchar('\b');
            }
            if (c >= 32 && pos < 127) {
                putchar(c);
                input[pos++] = c;
                input[pos] = 0;
            }
        }
        
        /* Parse and execute */
        Cmd cmd = parse_cmd(input);
        
        switch (cmd) {
            case CMD_HELP:
                cmd_help();
                break;
            case CMD_STATUS:
                cmd_status();
                break;
            case CMD_PEERS:
                cmd_peers();
                break;
            case CMD_CLEAR:
                draw_splash();
                break;
            case CMD_QUIT:
                puts("\x1B[33mGoodbye!\x1B[0m\n");
                return;
            case CMD_LOOKUP:
                cmd_lookup(input + 7);  /* skip "lookup " */
                break;
            case CMD_OPENCODE:
                puts("\x1B[32m[OPENCODE]\x1B[0m Starting OpenCode native...\n\n");
                break;
            case CMD_RUN:
                puts("\x1B[32m[RUN]\x1B[0m ");
                puts(input + 4);
                puts("\n\n");
                break;
            case CMD_PULL:
                puts("\x1B[32m[PULL]\x1B[0m Loading: ");
                puts(input + 5);
                puts("\n\n");
                break;
            case CMD_POD:
                puts("\x1B[32m[POD]\x1B[0m ");
                puts(input + 4);
                puts("\n\n");
                break;
            case CMD_NONE:
                if (pos > 0) {
                    /* Try lookup */
                    cmd_lookup(input);
                }
                break;
        }
    }
}

/* ═══════════════════════════════════════════════════════════════
 * ENTRY POINT
 * ================================================================*/

void main(void) {
    docs_init();
    main_loop();
}

/* ═══════════════════════════════════════════════════════════════
 * END OF MAXX-NANO TUI
 * Pure DOS-style terminal, .mdocx documents
 * ================================================================*/

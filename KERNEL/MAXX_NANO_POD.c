/*=============================================
 * MAXX-NANO: POD CORE + PEER DISCOVERY
 * 
 * Philosophy:
 * - Single agent, not multi-agent
 * - Can FIND other MAXX-nano nodes
 * - Can PULL assistance from peers
 * - Can ASSIST other peers
 * - Pod compilation target
 * 
 * Boot: MAXX → Find Peers → Run OpenCode (no OS)
 * =============================================================*/

#include <stdint.h>
#include <stdbool.h>

/* ═══════════════════════════════════════════════════════════════
 * HARDWARE I/O (minimal)
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
int strncmp(const char *a, const char *b, int n) { for (int i = 0; i < n; i++) { if (a[i] != b[i]) return a[i] - b[i]; if (!a[i]) break; } return 0; }
char *strchr(const char *s, int c) { while (*s) { if (*s == c) return (char *)s; s++; } return 0; }

/* ═══════════════════════════════════════════════════════════════
 * NETWORK / PEER DISCOVERY
 * Single node can find other MAXX-nano instances
 * ================================================================*/

#define MAX_PEERS 8
#define NODE_NAME_LEN 32

typedef struct {
    char name[NODE_NAME_LEN];
    char ip[16];
    int port;
    bool available;
    bool has_ollama;
    bool has_opencode;
    int capabilities;  /* bitmask: 1=ollama, 2=opencode, 4=web */
} PeerNode;

static PeerNode peers[MAX_PEERS];
static int peer_count = 0;
static char my_name[NODE_NAME_LEN] = "MAXX-nano-01";
static bool is_master = false;

/* Simulated peer discovery (would use UDP broadcast in real) */
void peer_discovery_scan(void) {
    puts("\n[DISCOVERY] Scanning for MAXX-nano peers...\n");
    
    /* Simulated peers for demo */
    /* In real: UDP broadcast on port 1967 */
    
    peer_count = 0;
    
    /* Simulated found peers */
    peer_add("MAXX-nano-02", "192.168.1.101", 1967, true, true, false, 3);
    peer_add("MAXX-nano-03", "192.168.1.102", 1967, true, false, true, 5);
    
    /* This node becomes master if no other master */
    is_master = true;
    
    puts("[DISCOVERY] Found ");
    putchar('0' + peer_count);
    puts(" peer(s)\n");
    puts("[STATUS] This node is: ");
    puts(is_master ? "MASTER\n" : "SLAVE\n");
}

void peer_add(const char *name, const char *ip, int port, bool avail, bool ollama, bool opencode, int cap) {
    if (peer_count >= MAX_PEERS) return;
    
    int i = 0;
    while (name[i] && i < NODE_NAME_LEN-1) { peers[peer_count].name[i] = name[i]; i++; }
    peers[peer_count].name[i] = 0;
    
    i = 0;
    while (ip[i] && i < 15) { peers[peer_count].ip[i] = ip[i]; i++; }
    peers[peer_count].ip[i] = 0;
    
    peers[peer_count].port = port;
    peers[peer_count].available = avail;
    peers[peer_count].has_ollama = ollama;
    peers[peer_count].has_opencode = opencode;
    peers[peer_count].capabilities = cap;
    
    peer_count++;
}

void peer_list(void) {
    puts("\n=== PEER NODES ===\n");
    for (int i = 0; i < peer_count; i++) {
        puts("  ");
        puts(peers[i].name);
        puts("\n    IP: ");
        puts(peers[i].ip);
        puts(":");
        /* Print port */
        int p = peers[i].port;
        putchar('0' + (p / 1000) % 10);
        putchar('0' + (p / 100) % 10);
        putchar('0' + (p / 10) % 10);
        putchar('0' + p % 10);
        puts("\n    Status: ");
        puts(peers[i].available ? "ONLINE" : "OFFLINE");
        puts("\n    Capabilities: ");
        if (peers[i].has_ollama) putchar('O');  /* Ollama */
        if (peers[i].has_opencode) putchar('C'); /* OpenCode */
        putchar('\n');
    }
    puts("==================\n\n");
}

/* ═══════════════════════════════════════════════════════════════
 * PEER ASSISTANCE - Pull from other nodes
 * ================================================================*/

void peer_pull(const char *peer_name, const char *resource) {
    /* Find peer */
    int peer_idx = -1;
    for (int i = 0; i < peer_count; i++) {
        if (strcmp(peers[i].name, peer_name) == 0) {
            peer_idx = i;
            break;
        }
    }
    
    if (peer_idx < 0) {
        puts("[ERROR] Peer not found: ");
        puts(peer_name);
        puts("\n");
        return;
    }
    
    PeerNode *p = &peers[peer_idx];
    
    if (!p->available) {
        puts("[ERROR] Peer is offline\n");
        return;
    }
    
    puts("\n[PULL] Requesting ");
    puts(resource);
    puts(" from ");
    puts(p->name);
    puts("\n");
    
    /* Check capabilities */
    if (strcmp(resource, "ollama") == 0) {
        if (p->has_ollama) {
            puts("[OK] Peer has Ollama - streaming model...\n");
            /* Would curl/wget from peer */
        } else {
            puts("[WARN] Peer doesn't have Ollama\n");
        }
    }
    else if (strcmp(resource, "opencode") == 0) {
        if (p->has_opencode) {
            puts("[OK] Peer has OpenCode - pulling...\n");
            /* Would git clone or rsync from peer */
        } else {
            puts("[WARN] Peer doesn't have OpenCode\n");
        }
    }
    else if (strcmp(resource, "assist") == 0) {
        puts("[OK] Requesting assistance from peer...\n");
        puts("     (Peer would run computation and return result)\n");
    }
}

/* ═══════════════════════════════════════════════════════════════
 * ASSIST MODE - This node can help others
 * ================================================================*/

void assist_listen(void) {
    puts("\n[ASSIST] Listening for peer requests on port 1967...\n");
    puts("         (Would accept: model requests, code execution)\n");
    puts("         This node can provide:\n");
    puts("           - Ollama inference\n");
    puts("           - OpenCode tools\n");
    puts("           - Webhook relay\n");
    puts("           - File sharing\n\n");
}

/* ═══════════════════════════════════════════════════════════════
 * POD FORMATION - Multiple nodes work together
 * ================================================================*/

typedef enum {
    POD_NONE,
    POD_SEEKING,    /* Looking for pod */
    POD_FORMING,    /* Forming pod */
    POD_JOINED,     /* Joined pod */
    POD_MASTER      /* This node is master */
} PodState;

static PodState pod_state = POD_NONE;

void pod_discover(void) {
    puts("\n[POD] Discovering pod...\n");
    peer_discovery_scan();
    
    if (peer_count > 0) {
        pod_state = POD_JOINED;
        puts("[POD] Joined pod with ");
        putchar('0' + peer_count);
        puts(" peer(s)\n");
    } else {
        pod_state = POD_MASTER;
        puts("[POD] No peers found. Becoming pod MASTER.\n");
    }
}

void pod_status(void) {
    puts("\n=== POD STATUS ===\n");
    puts("  This node: ");
    puts(my_name);
    puts("\n");
    puts("  State: ");
    switch (pod_state) {
        case POD_NONE: puts("Idle"); break;
        case POD_SEEKING: puts("Seeking peers"); break;
        case POD_FORMING: puts("Forming pod"); break;
        case POD_JOINED: puts("Joined pod"); break;
        case POD_MASTER: puts("Pod MASTER"); break;
    }
    putchar('\n');
    puts("  Peers: ");
    putchar('0' + peer_count);
    putchar('\n');
    puts("=================\n\n");
}

/* ═══════════════════════════════════════════════════════════════
 * TRIGGER SYSTEM (enhanced with pod)
 * ================================================================*/

typedef enum {
    TRIGGER_NONE,
    TRIGGER_RUN,
    TRIGGER_PULL,
    TRIGGER_PEER,      /* NEW: peer-related */
    TRIGGER_POD,       /* NEW: pod-related */
    TRIGGER_ASSIST,    /* NEW: assist mode */
    TRIGGER_LOOKUP,
    TRIGGER_LOOP,
    TRIGGER_CHECK,
    TRIGGER_HELP,
    TRIGGER_STATUS,
    TRIGGER_CONNECT,
    TRIGGER_OPENCODE   /* NEW: run OpenCode */
} TriggerType;

typedef struct {
    TriggerType type;
    char target[64];
    char args[128];
} MAXX_Trigger;

MAXX_Trigger parse_trigger(const char *input) {
    MAXX_Trigger t = { .type = TRIGGER_NONE };
    
    while (*input == ' ') input++;
    
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
    else if (strncmp(input, "peer ", 5) == 0) {
        t.type = TRIGGER_PEER;
        int i = 0; input += 5;
        while (*input && i < 63) t.target[i++] = *input++;
    }
    else if (strncmp(input, "pod ", 4) == 0) {
        t.type = TRIGGER_POD;
        int i = 0; input += 4;
        while (*input && i < 63) t.target[i++] = *input++;
    }
    else if (strncmp(input, "assist", 6) == 0) {
        t.type = TRIGGER_ASSIST;
    }
    else if (strncmp(input, "opencode", 8) == 0) {
        t.type = TRIGGER_OPENCODE;
    }
    else if (strncmp(input, "lookup ", 7) == 0) {
        t.type = TRIGGER_LOOKUP;
        int i = 0; input += 7;
        while (*input && i < 63) t.target[i++] = *input++;
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
    
    return t;
}

void respond(MAXX_Trigger *t) {
    switch (t->type) {
        case TRIGGER_HELP:
            puts("\n=== MAXX-NANO COMMANDS ===\n");
            puts("  run <cmd>      - Execute a command\n");
            puts("  pull <tool>    - Load a tool locally\n");
            puts("  peer <name>    - Interact with peer\n");
            puts("  pod discover   - Find and join pod\n");
            puts("  pod status     - Show pod status\n");
            puts("  assist         - Enable assist mode\n");
            puts("  opencode       - Run OpenCode\n");
            puts("  lookup <kw>   - Search knowledge\n");
            puts("  loop <cond>    - Loop until done\n");
            puts("  status         - Show system status\n");
            puts("===========================\n\n");
            break;
            
        case TRIGGER_STATUS:
            pod_status();
            break;
            
        case TRIGGER_POD:
            if (strcmp(t->target, "discover") == 0) {
                pod_discover();
            } else if (strcmp(t->target, "status") == 0) {
                pod_status();
            }
            break;
            
        case TRIGGER_PEER:
            /* Parse: peer list / peer pull <name> <resource> */
            if (strcmp(t->target, "list") == 0) {
                peer_list();
            } else {
                /* Try to pull from peer */
                /* Format: peer <peername> <resource> */
                char *space = strchr(t->target, ' ');
                if (space) {
                    *space = 0;
                    peer_pull(t->target, space + 1);
                }
            }
            break;
            
        case TRIGGER_ASSIST:
            assist_listen();
            break;
            
        case TRIGGER_OPENCODE:
            puts("\n[OPENCODE] Starting OpenCode native...\n");
            puts("           No OS required. Running on MAXX.\n");
            puts("           (In real: exec OpenCode binary)\n\n");
            break;
            
        case TRIGGER_RUN:
            puts("\n[RUN] Executing: ");
            puts(t->target);
            puts("\n");
            break;
            
        case TRIGGER_PULL:
            puts("\n[PULL] Loading: ");
            puts(t->target);
            puts("\n");
            break;
            
        case TRIGGER_LOOKUP:
            puts("\n[LOOKUP] Searching for: ");
            puts(t->target);
            puts("\n");
            break;
            
        default:
            puts("\n[UNKNOWN] Type 'help' for commands\n");
    }
}

/* ═══════════════════════════════════════════════════════════════
 * MAIN SHELL LOOP
 * ================================================================*/

void shell_loop(void) {
    char input[256] = {0};
    int pos = 0;
    
    /* Auto-discover pod on startup */
    pod_discover();
    
    puts("\nMAXX-nano> ");
    
    while (1) {
        char c = getch();
        putchar(c);
        
        if (c == '\r') {
            putchar('\n');
            
            if (pos > 0) {
                MAXX_Trigger t = parse_trigger(input);
                respond(&t);
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
    puts("║     MAXX-nano v0.02 - POD CORE                 ║\n");
    puts("║     Single Agent | Pod Discovery | Native      ║\n");
    puts("║     Size: < 64KB | Boot: USB/Floppy            ║\n");
    puts("╚═══════════════════════════════════════════════════╝\n");
    puts("\nType 'help' for commands.\n");
    puts("Try: pod discover\n");
    puts("Try: peer list\n");
    puts("Try: opencode\n\n");
    
    shell_loop();
}

/* ═══════════════════════════════════════════════════════════════
 * END OF MAXX-NANO POD CORE
 * Can find peers, form pods, run OpenCode native
 * ================================================================*/

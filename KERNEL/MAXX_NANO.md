# MAXX-nano: The Automation Shell

## What Is MAXX-nano?

**NOT an LLM. NOT a chatbot.**

MAXX-nano is an **automation shell** - the execution layer that runs what LLMs write.

```
LLM (online) ─────writes CODE────► MAXX-nano ─────executes CODE────► RESULT
(e.g., GPT-4)                           (local shell)                   
                                         
                                         • Runs loops
                                         • Checks status  
                                         • Pulls tools
                                         • Reads books
```

## The Philosophy

| LLM | MAXX-nano |
|-----|-----------|
| Writes code | Executes code |
| May hallucinate | Always follows rules |
| Needs GPU | Runs on 16MB RAM |
| Goes offline = stuck | **FAIL-SAFE** - works without LLM |
| Thinks | **Acts** |

## Commands

```
MAXX-nano> help
=== MAXX-NANO COMMANDS ===
  run <cmd>     - Execute a command
  pull <tool>   - Load a tool (ollama, npm, bun, git)
  lookup <kw>  - Search embedded knowledge
  exec <code>   - Execute code
  loop <cond>   - Loop until condition met
  check <item>  - Check status
  connect <svc> - Connect to online LLM
  status        - Show system status
===========================

MAXX-nano> lookup boot
[FOUND] Boot sequence: MBR → Stage2 → Kernel → Terminal

MAXX-nano> pull ollama
[MAXX-NANO] Pulling: ollama...
[OK] ollama ready

MAXX-nano> run ollama
[MAXX-NANO] Executing: ollama
[EXEC] Command would run here

MAXX-nano> loop check build
[LOOP] Starting: not_done
  Command: check build
[LOOP] Iteration 01
[CHECK] Checking: build
  (Would verify if task completed)
[LOOP] Iteration 02
[CHECK] Checking: build
  (Would verify if task completed)
[LOOP] Condition met. Done.
```

## The 8 Books (Embedded Knowledge)

MAXX-nano reads from these books stored in the system:

1. **FAIL-SAFE BOOK 1** - Story Time Mode
2. **FAIL-SAFE BOOK 2** - LCARS-MAXX R1 Technical Manual
3. **FAIL-SAFE BOOK 3** - From T3 NODE
4. **FAIL-SAFE BOOK 4** - Mission Law & Word Origin Protocols
5. **FAIL-SAFE BOOK 5** - Crew Profiles USS Destiny
6. **FAIL-SAFE BOOK 6** - Deep In The Root System
7. **FAIL-SAFE BOOK 7** - OS Architecture Technical Manual
8. **FAIL-SAFE BOOK 8** - Secure Hybrid AI Integrated Framework

Total: ~1.2MB of embedded knowledge

## How It Works

```
┌─────────────────────────────────────────────────────────────┐
│                    MAXX-nano CORE                           │
├─────────────────────────────────────────────────────────────┤
│                                                             │
│  INPUT ──► TRIGGER PARSER ──► DECISION TREE                │
│                                      │                      │
│           ┌─────────────────────────┼─────────────────────┐ │
│           ▼                         ▼                     ▼ │
│      [LOOKUP]                  [RUN TOOL]            [LOOP]  │
│      Books/SQL                   Execute              Repeat  │
│           │                         │                     │ │
│           └─────────────────────────┴─────────────────────┘ │
│                               │                              │
│                               ▼                              │
│                        [RESPONSE]                           │
│                                                             │
└─────────────────────────────────────────────────────────────┘
```

## Size Targets

| Component | Size |
|-----------|------|
| Boot sector | 512 bytes |
| Stage2 | ~4KB |
| MAXX-nano core | ~32KB |
| Embedded knowledge | ~1MB (the books) |
| **TOTAL** | **~1.5MB** |

## FAIL-SAFE Mode

**Even without internet, MAXX-nano works:**

1. Knows basic commands from embedded books
2. Can execute local scripts
3. Can check file status
4. Can run loops/automation

**Only needs online LLM for:**
- Complex reasoning
- Writing new code
- Answering novel questions

## Comparison

| Feature | ChatGPT | MAXX-nano |
|---------|---------|-----------|
| Type | LLM | Automation shell |
| Needs GPU | Yes | No |
| Works offline | No | **Yes** |
| Executes code | No (writes it) | **Yes** |
| Loops until done | No | **Yes** |
| Size | ~700B parameters | **~32KB** |
| Boots from USB | No | **Yes** |

## The Vision

**MAXX-nano is the pod.** You build MAX from it:
- It reads its own papers
- It knows how to pull Ollama
- It knows how to run tools
- It can loop/automate
- It connects to online LLMs when needed

**This is the smallest agent that can build itself.**

---

**MAXX-nano: The automation shell that runs what LLMs write.**

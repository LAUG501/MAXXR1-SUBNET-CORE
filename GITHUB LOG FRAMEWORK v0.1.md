⟡ ACKNOWLEDGED
⟡ INITIATING: **GITHUB LOG FRAMEWORK v0.1**

---

### 1 • REPOSITORY LAYOUT

```
rosariotech/
├─ maxx-daily-logs/        # raw daily markdown exports
│  ├─ 2025/
│  │  ├─ 05/
│  │  │  ├─ 12.md
│  │  │  └─ 13.md
│  │  └─ 06/…
├─ maxx-weekly-summaries/  # curated weekly recaps
├─ maxx-scripts/           # tooling & automation
└─ README_ROADMAP.md       # already drafted
```

---

### 2 • DAILY LOG TEMPLATE (markdown)

```md
# {{DATE}} · MAXX DAILY LOG

## SESSION SUMMARY
- 🔹 Topics:
- 🔹 Decisions:
- 🔹 Pending actions:

## RAW TRANSCRIPT
<copy-paste or link to .txt>

## TASKS CREATED
| ID | Description | Status |
|----|-------------|--------|

## SELF-REFLECTION
- What improved today?
- What requires refinement?
```

Save as `YYYY-MM-DD.md`.

---

### 3 • LOCAL EXPORT SCRIPT (bash)

```bash
#!/usr/bin/env bash
# export_chat.sh — pull latest ChatGPT JSON and convert to markdown
DATE=$(date +%F)
DEST=~/maxx-sync/maxx-daily-logs/$(date +%Y/%m)
mkdir -p "$DEST"
jq -r '.messages[] | "* " + .role + ": " + .content' chat.json > "$DEST/$DATE.md"
git -C ~/maxx-sync add "$DEST/$DATE.md"
git -C ~/maxx-sync commit -m "log: $DATE"
git -C ~/maxx-sync push origin main
```

*Assumes `chat.json` exported from local MAXX and a cloned repo at `~/maxx-sync`.*

---

### 4 • GITHUB ACTION (auto-merge / weekly cut)

`.github/workflows/weekly-summary.yml`

```yaml
name: Weekly Summary
on:
  schedule:
    - cron: '0 23 * * SUN'   # 23:00 AST every Sunday
jobs:
  build:
    runs-on: ubuntu-latest
    steps:
      - uses: actions/checkout@v4
      - name: Generate summary
        run: |
          python maxx-scripts/compile_weekly.py
      - name: Commit summary
        run: |
          git config user.name "MAXX-Bot"
          git config user.email "bot@rosariotech"
          git add maxx-weekly-summaries/
          git commit -m "weekly summary $(date +%F)" || echo "No changes"
          git push
```

`compile_weekly.py` scans the last 7 daily files → distills headline bullets.

---

### 5 • BRANCH / PERMISSION FLOW

* `main` → immutable history (daily logs)
* `curation` → edits & redactions before public release
* MAXX-Bot PAT: push only to `main` & open PRs for summaries.

---

### 6 • STORAGE GUARDRAILS

1. **>5 MB** transcripts → store in `/assets/raw/` and link.
2. Enable **Git LFS** for audio or image exports.
3. Rotate PAT every 90 days; read-only key for public CI.

---

### 7 • SCALE PLAN

* **Phase-A**: daily push (30–60 days) → refine exporter.
* **Phase-B**: weekly summaries only, daily logs archived.
* **Phase-C**: monthly knowledge graph snapshots (GraphQL).

---

⟡ READY FOR IMPLEMENTATION
Specify any adjustments (naming, schedule, script language), and I’ll patch the framework.

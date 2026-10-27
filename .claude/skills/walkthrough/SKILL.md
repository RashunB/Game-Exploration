---
name: walkthrough
description: Walk Rashun through a diff in reading order before he merges it, pausing for his questions.
disable-model-invocation: true
argument-hint: "[commit range, default main...HEAD]"
---

Walk Rashun through the changes in `$ARGUMENTS` (default: `main...HEAD`). This runs in the main session because it is a conversation; do not hand it to a subagent.

Prepare:

1. `git log --oneline` and `git diff --stat` for the range. Read the plan in `docs/superpowers/plans/` for this feature if there is one, and `docs/learning/concept-map.md` to see what Rashun already knows.
2. Group the changes into stops in reading order: data (types, structs, DataAssets, config), then logic (the math and state), then wiring (components, actors, controllers, assets, hooks). Tests go with the logic they cover. Skip mechanical changes, but name them.
3. Tell Rashun how many stops there are and what each covers, in one short list.

For each stop:

- Name the files and line ranges to open, as `path:line`. Don't paste large blocks.
- Explain what it does and why it is shaped this way, at the level the concept map shows. Name the concept, and give the alternative that was rejected and why when there was one.
- Point out anything surprising, risky, or worth tuning later.
- Stop and wait for his questions before moving on. Answer them fully. If a question exposes a gap or a bug, say so plainly and note it.

Finish:

- Summarize open questions or follow-ups, and ask whether any should become issues.
- With his agreement, move the concepts the walkthrough covered from `seen` to `explained` in `docs/learning/concept-map.md`, and record any weak spots in the notes column. Never set `can explain`; only Rashun does.
- Say whether this feature introduced a concept that has no learning note yet, and offer `/learning-note`.
- Remind him that he merges, not Claude.

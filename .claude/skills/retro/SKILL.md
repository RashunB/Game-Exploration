---
name: retro
description: Draft the milestone retro for Rashun to edit, with the tutor subagent.
disable-model-invocation: true
argument-hint: "[milestone, e.g. M2]"
context: fork
agent: tutor
---

Draft the retro for milestone $ARGUMENTS (default: the milestone most recently gated in `PLAN.md` section 8) as `docs/learning/retros/<milestone>.md`.

Sources: `git log` for the milestone's commits, `PLAN.md` section 8, `CHANGELOG.md`, the learning notes and concept-map rows touched in the milestone, and `docs/playtests/` entries in its date range.

Sections: what was built; concepts introduced, with links to their notes; what Rashun can now explain on his own (only rows he set to `can explain`); what is still opaque (rows still `seen` or `explained`, plus quiz weak spots); your-turn tasks done and skipped; process notes (what slowed the cycle down). Mark the file as a draft at the top. Report what you wrote.

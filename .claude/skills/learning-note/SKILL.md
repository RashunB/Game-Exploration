---
name: learning-note
description: Write a learning note for a feature or commit range with the tutor subagent. Use after a walkthrough when the feature introduced a concept with no note yet, or for a backlog note, once Rashun agrees.
argument-hint: "[commit range or note name, e.g. 0003-scoring-component-and-config]"
context: fork
agent: tutor
---

Write the next learning note for: $ARGUMENTS

If that is a backlog note name from the table in `docs/learning/README.md`, cover exactly the scope listed there and in its GitHub issue. If it is a commit range, find the one concept that range introduces that has no note yet; if every concept already has a note, say so and write nothing.

Follow `docs/learning/notes/TEMPLATE.md` and your rules. Then update the concept map, the glossary, and the backlog table, and report the files you wrote and any gaps.

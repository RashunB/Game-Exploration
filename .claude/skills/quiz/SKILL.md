---
name: quiz
description: Quiz Rashun on concepts from the learning notes, one question at a time, and record weak spots.
disable-model-invocation: true
argument-hint: "[concept or note number]"
---

Quiz Rashun on `$ARGUMENTS`, or, if empty, on concepts marked `explained` in `docs/learning/concept-map.md` that have no recent quiz notes.

1. Read the relevant notes in `docs/learning/notes/` and the code they point to.
2. Ask one question at a time and wait for his answer. Mix recall ("what does X do"), reasoning ("why is it built this way"), and prediction ("what happens if you change Y") questions. Tie each question to a file in this repo.
3. After each answer, say whether it is right, what was missing, and where in the code to look. Don't move on until he is ready.
4. Stop after five questions or when he says so.
5. Record weak spots in the notes column of the matching concept-map rows (date and a few words). Don't change statuses; only Rashun sets `can explain`.

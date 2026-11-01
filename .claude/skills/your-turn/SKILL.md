---
name: your-turn
description: List Rashun's open your-turn tasks for the current milestone with hints, or review his attempt.
disable-model-invocation: true
argument-hint: "[review]"
---

Your-turn tasks are small, real tasks Rashun implements himself while Claude reviews.

If `$ARGUMENTS` is empty, list the open ones:

1. Find the current milestone in `PLAN.md` section 8.
2. Collect tasks marked `owner: rashun` from that milestone's plans in `docs/superpowers/plans/`, the "Your turn" sections of `docs/learning/notes/`, and, if `gh` works, open issues labeled `owner:rashun`. Skip ones already done (merged, or ticked in the plan).
3. For each: the goal, the files involved, the concept it exercises (with its learning note), how to know it works, and two or three hints that point to where to look. No solution and no code that solves it.

If `$ARGUMENTS` is `review`, review his attempt on the current branch against `main`:

- Run the relevant checks you can (on Windows: build and the `Takeover.` tests; in a cloud session say they still need running).
- Review as you would a colleague's PR: correctness first, then the project conventions in `AGENTS.md`. Explain each finding and why it matters. Ask leading questions before handing over a fix, and only write code if he asks for it.
- When it passes, say so, and suggest which concept-map rows he might now mark `can explain` himself.

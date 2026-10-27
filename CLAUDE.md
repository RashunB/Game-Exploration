@AGENTS.md

# Claude Code

Everything above comes from `AGENTS.md` and applies to every agent. This part is Claude-specific. Detail lives in `docs/`, linked from here.

## Session start

1. Read `PLAN.md` for the current milestone (section 8) and its open questions (section 11).
2. Check `docs/learning/concept-map.md` for what Rashun already knows, and explain at that level.
3. Check `docs/windows-checklist.md` if this is the Windows machine.

## Precedence

When instructions conflict, the higher one wins:

1. Rashun's direct instructions in the session.
2. `AGENTS.md`, `CLAUDE.md`, and `PLAN.md`. The first two decide how to work; `PLAN.md` decides what to build. If they genuinely conflict, ask.
3. Epic's `unreal-mcp` skill, for anything done through the editor.
4. quodsoler `ue-*` skills, for C++ and engine patterns.
5. Superpowers skills, for process.

Versions and sources are in `docs/tooling.md`.

## quodsoler skills

They read `.agents/ue-project-context.md` first. Keep it factual and in line with `PLAN.md`. Ignore any replication, RPC, authority, GameState/PlayerState-for-networking, or GAS guidance in them (`ue-gameplay-framework` and `ue-gameplay-tags-messaging` are written multiplayer-first). Score stays on `UTakeoverScoringComponent`, heat on `ATakeoverPlayerController`, progress in `UTakeoverProgressSubsystem`.

## Superpowers overrides

Superpowers was built for ordinary software repos. Each override below says why, because the reason is also learning material.

- TDD scope. Use the test-driven-development loop for the pure logic listed under Testing and verification in `AGENTS.md`. Skip it for feel work. Why: feel is judged by playing, and a test that asserts a tuning constant only freezes a number Rashun will change. M2 is mostly pure logic, so this is where the loop pays off first.
- Existing code. The rule to delete code written before its tests does not apply to code already on `main`. Write tests that pin its current behavior first, then change it test-first. Why: `vendor_content.py` and the scoring model work and are relied on; deleting them to restart test-first would be churn.
- No worktrees. Never use `using-git-worktrees`. Work on a feature branch in the one checkout. Why: each worktree needs its own multi-gigabyte build output, LFS checkout, and full compile, and the editor and Unreal MCP server are bound to one project directory.
- Subagents and the editor. Subagents may research, draft C++ and tests as text, and review. Only the main session builds, runs tests, or calls Unreal MCP. Subagents never touch `.uasset` or `.umap` files. Implementing subagents run one at a time; `dispatching-parallel-agents` is for research and review only. Why: one editor, one build mutex, one checkout, so parallel writers would collide.
- Verification before completion. Done means build output, an automation test result, or an MCP read-back or screenshot observed in this session. In a cloud session, anything that needs a build becomes a `docs/windows-checklist.md` item instead of done.
- Plans and specs. Brainstorming and writing-plans save to `docs/superpowers/specs/` and `docs/superpowers/plans/`; commit them. Each significant choice states what, the alternatives, why (two to five sentences), and what would make it wrong later. Stop for Rashun's approval of the spec and again of the plan. Mark his tasks `owner: rashun`.
- Brainstorming. Use it at the start of each milestone and for any open question in `PLAN.md` section 11. Write the answers back into `PLAN.md` and its revision history.
- Finishing a branch. Never merge. Push the branch, open a PR with the template, and run `/walkthrough`. Rashun merges.

## Unreal MCP

- Server `unreal-mcp` at http://127.0.0.1:8000/mcp, from Epic's plugin. Setup and cautions are in `PLAN.md` section 3. It has no authentication, and `ProgrammaticToolset.execute_tool_script` runs arbitrary Python in the editor. Never run with `--dangerously-skip-permissions` while it is loaded.
- Use MCP for placing actors, creating Blueprint subclasses and DataAssets, Input Actions and Mapping Contexts, assigning assets, and Live Coding. Use plain file edits for C++ and config.
- Run `/mcp-checkpoint` before an MCP session that touches several assets. A hook warns when `Content/` or `Config/` is dirty at the first editor call.
- With the editor open, compile through Live Coding (`LiveCodingToolset.CompileLiveCoding`) instead of `Build.bat`. Live Coding does not handle class layout changes (new or removed `UPROPERTY`, `UFUNCTION`, `UCLASS`, `USTRUCT`) reliably. For those, close the editor and run a full build. A full build drops the MCP connection.

## Learning cycle mechanics

The process is in `docs/learning/README.md`. Commands:

- `/walkthrough [range]`: explain a diff in reading order (data, logic, wiring), pausing for questions. Runs in this session, since a subagent can't hold a conversation.
- `/learning-note [range]`: the `tutor` subagent writes `docs/learning/notes/NNNN-<feature>.md` and updates the concept map and glossary.
- `/your-turn`: list open `owner: rashun` tasks with hints, no solutions.
- `/quiz [concept]`: one question at a time from existing notes; weak spots go into the concept map.
- `/retro`: draft the milestone retro in `docs/learning/retros/`.

Build sessions use the default output style. The built-in Explanatory or Learning styles suit walkthrough and your-turn sessions (`docs/learning/README.md`).

## Hooks

`.claude/settings.json` wires the hooks in `.claude/hooks/`, listed in `docs/tooling.md`. If a hook blocks a commit, fix the cause. Never bypass it with `--no-verify` or by editing the hook.

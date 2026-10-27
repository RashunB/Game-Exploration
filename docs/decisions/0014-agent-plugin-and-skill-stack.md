# 0014: Agent plugin and skill stack, with the no-worktrees override

- Status: accepted
- Date: 2026-10-27

## Context

Claude Code writes most of the code, and Rashun learns from it. Three gaps: engine-correct C++ patterns, a disciplined process (specs, plans, TDD, verification), and editor control. Each has an off-the-shelf answer, built for different contexts.

## Decision

Three layers, one job each, with a fixed precedence in `CLAUDE.md`: Rashun, then `AGENTS.md`/`CLAUDE.md`/`PLAN.md`, then Epic's `unreal-mcp` skill for editor work, then quodsoler `ue-*` skills for C++ patterns, then Superpowers for process.

- Editor control: Epic's `unreal-engine-skills-for-claude-code` plugin (already in use).
- C++ knowledge: 26 of the 31 `quodsoler/unreal-engine-skills`, vendored at a pinned commit. Networking, GAS, Mover, Mass, and Game Features are left out because the game uses none of them.
- Process: the full Superpowers plugin, on every model.

Overrides for an Unreal project, each with its reason in `CLAUDE.md`: no git worktrees (each needs its own multi-gigabyte build and LFS checkout, and the editor and MCP server bind to one directory); TDD only for pure logic; tests pinning existing code instead of deleting it; only the main session builds, tests, or calls MCP; implementing subagents run one at a time; plans and specs record their reasoning and stop for approval; Claude never merges.

`AGENTS.md` holds tool-agnostic rules, and `CLAUDE.md` imports it, so the repo isn't tied to one agent.

## Alternatives

- No process plugin: relies on per-session prompting for discipline.
- All 31 quodsoler skills with overrides: more always-loaded context, and overrides only might stop multiplayer-first advice; an uninstalled skill can't give it.
- Installing quodsoler skills with `npx skills add`: runs a package from npm; a pinned clone does the same and can be read first.
- Thinner Superpowers on stronger models: it would be an instruction the model has to remember, not a setting.

## Consequences

About 17.6 KB of skill descriptions load every turn. Superpowers follows the marketplace, so its version can move between sessions; `docs/tooling.md` records what was enabled. Updating any third-party layer is ask-first. Hooks enforce the license boundary and the MCP checkpoint, since instructions alone aren't guaranteed.

## Revisit if

Superpowers or the quodsoler skills start contradicting `PLAN.md` often enough that overrides become the bulk of `CLAUDE.md`, or Epic's plugin grows the C++ guidance itself.

## Sources

The 2026-10-27 admin brief and session, `docs/tooling.md`, `CLAUDE.md`.

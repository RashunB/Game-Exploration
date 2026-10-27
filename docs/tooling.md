# Tooling

What the agent tooling in this repo is, where each piece came from, and how to update it. Updating any third-party piece is an ask-first change (`AGENTS.md`).

Recorded 2026-10-27.

## Layers

| Layer | Source | Job | How it is installed |
| --- | --- | --- | --- |
| Editor control | Epic `unreal-engine-skills-for-claude-code@claude-plugins-official` | Drive the running editor over Unreal MCP: actors, Blueprint subclasses, DataAssets, Input assets, Live Coding, automation tests, screenshots, logs | Enabled in `.claude/settings.json` |
| Unreal C++ knowledge | [quodsoler/unreal-engine-skills](https://github.com/quodsoler/unreal-engine-skills), MIT | Engine patterns for C++ gameplay, physics, AI, animation, rendering | Vendored into `.claude/skills/ue-*` |
| Process | [obra/superpowers](https://github.com/obra/superpowers) (`superpowers@claude-plugins-official`), MIT | Brainstorm, spec, plan, execution, TDD, debugging, verification, code review | Enabled in `.claude/settings.json` |

`CLAUDE.md` sets the precedence between them and the Superpowers overrides for this project.

## Epic plugin

- Id: `unreal-engine-skills-for-claude-code@claude-plugins-official`.
- Version: not recorded yet. Read it from `/plugin` on the Windows machine and fill it in here (`docs/windows-checklist.md`).
- Engine side: the `ModelContextProtocol`, `AllToolsets`, and `LiveCodingToolset` plugins in `Takeover.uproject`, Editor target only. See `PLAN.md` section 3.
- Its SessionStart hook needs Git Bash on Windows.

## Superpowers

- Id: `superpowers@claude-plugins-official`, as given in the Superpowers README.
- Version at enablement: 7.0.1, upstream commit `2ce8aa1` (2026-10-21).
- `enabledPlugins` does not pin a version. The plugin follows the marketplace, so a later session can load a newer release. Check the version with `/plugin` when behavior changes, and update this file.
- Skills: brainstorming, writing-plans, executing-plans, subagent-driven-development, dispatching-parallel-agents, test-driven-development, systematic-debugging, verification-before-completion, requesting-code-review, receiving-code-review, using-git-worktrees, finishing-a-development-branch, writing-skills, using-superpowers, diagnosing-superpowers.
- It adds a SessionStart hook (`run-hook.cmd session-start`, run through bash) that loads `using-superpowers`.
- Specs go to `docs/superpowers/specs/YYYY-MM-DD-<topic>-design.md` and plans to `docs/superpowers/plans/YYYY-MM-DD-<feature>.md`. Both are committed.
- `using-superpowers` states that CLAUDE.md and AGENTS.md take precedence over its skills, so the overrides in `CLAUDE.md` are expected use.
- Model: the full plugin runs on every model. Thinning it per model would be an instruction the model has to remember, not a setting, and M2 is mostly pure logic, where the plan and TDD steps pay off.

## quodsoler/unreal-engine-skills

- Upstream commit: `f3742d7b688690810df369802b90430324e380b9` (2026-09-28, "Updated skills for Unreal Engine 5.8. Added new skills."). The upstream README lists 31 skills; the repo description still says 27.
- License: MIT. The notice is in `.claude/skills/LICENSE-quodsoler-unreal-engine-skills.txt`.
- Files are copied verbatim. Do not edit them; put overrides in `CLAUDE.md` or `.agents/ue-project-context.md`.
- Installed (26): ue-actor-component-architecture, ue-ai-navigation, ue-animation-system, ue-async-threading, ue-audio-system, ue-blueprint-cpp-interop, ue-character-movement, ue-cpp-foundations, ue-data-assets-tables, ue-editor-tools, ue-gameplay-cameras, ue-gameplay-framework, ue-gameplay-tags-messaging, ue-input-system, ue-materials-rendering, ue-module-build-system, ue-niagara-effects, ue-physics-collision, ue-procedural-generation, ue-project-context, ue-sequencer-cinematics, ue-serialization-savegames, ue-state-trees, ue-testing-debugging, ue-ui-umg-slate, ue-world-level-streaming.
- Left out (5), approved 2026-10-27: ue-networking-replication, ue-gameplay-abilities (GAS), ue-mover (rollback-networked movement, Experimental), ue-mass-entity, ue-game-features. The game is single-player with no GAS, no Mass crowd, and no Game Feature plugins (`PLAN.md` section 2). An uninstalled skill cannot pull the agent toward those patterns; an override only might.
- The installed skills' descriptions total about 17.6 KB and load into context on every turn.

### Review findings (2026-10-27)

Method: read every kept `SKILL.md` frontmatter and its context section, the full `ue-project-context` and `ue-gameplay-framework` skills, and searched all 26 skills and their `references/` files for scripts, shell commands, URLs, file writes, and networking or GAS guidance. The full 13,000 lines were not read line by line.

- No scripts, hooks, or URLs. Every file is Markdown. No skill sets `allowed-tools`, `hooks`, `context`, or `agent` in its frontmatter.
- Shell commands appear only as examples: `UnrealEditor-Cmd.exe ... Automation RunTests` and Unreal Insights trace flags in `ue-testing-debugging`. Nothing tells the agent to fetch or install anything.
- Every skill reads `.agents/ue-project-context.md` first. That file is committed and states single-player, no replication, no GAS. `ue-project-context` writes it; re-run that skill only to refresh it, and keep the facts in line with `PLAN.md`.
- Contradicts `PLAN.md`: `ue-gameplay-framework` is written multiplayer-first. It puts scores and match state on a replicated GameState and per-player stats on PlayerState. This project keeps score on `UTakeoverScoringComponent`, heat on `ATakeoverPlayerController`, and progress in `UTakeoverProgressSubsystem` (`PLAN.md` section 4). `ue-gameplay-tags-messaging` and `ue-character-movement` also carry replication guidance. `CLAUDE.md` tells the agent to ignore all of it.
- The kept skills cross-reference the five removed skills about 70 times ("for replication, see ue-networking-replication"). Those links lead nowhere. Harmless.

### Updating

Ask first. Then:

```bash
git clone https://github.com/quodsoler/unreal-engine-skills.git /tmp/ues
git -C /tmp/ues log --oneline -5
diff -r --strip-trailing-cr /tmp/ues/skills/ue-cpp-foundations .claude/skills/ue-cpp-foundations   # repeat per skill, read every change
```

Some upstream files use CRLF line endings; `.gitattributes` normalizes them to LF on commit, hence `--strip-trailing-cr`. Copy the same 26 folders over the vendored ones, review the diff with `git diff --stat .claude/skills` and by reading it, rerun the review above, and update the commit hash and findings here.

`npx -y skills add quodsoler/unreal-engine-skills --agent claude-code` also installs them, but it runs a package from npm. A pinned clone does the same job and can be read before anything lands.

## Hooks

Hook scripts run under Git Bash on Windows and plain bash elsewhere. `.gitattributes` keeps them LF.

| Hook | Where | What it does |
| --- | --- | --- |
| Session start | `.claude/hooks/session-start.sh` (Claude Code SessionStart) | Points `core.hooksPath` at `Tools/githooks` if it is unset, so the git hooks below protect every session, cloud ones included |
| Commit guard | `.claude/hooks/guard-commit.sh` (PreToolUse on `Bash(git commit *)` and `Bash(git add *)`) | Blocks a commit that stages files under `Content/` outside the three first-party folders, and a force-add under `Content/`. Reminds the agent to run `vendor_content.py verify` when level files are staged |
| MCP checkpoint | `.claude/hooks/mcp-checkpoint.sh` (PreToolUse on Unreal MCP tools) | Warns once per session when `Content/` or `Config/` has uncommitted changes before an editor call. Does not block |
| Tutor write guard | `.claude/hooks/tutor-write-guard.sh` (PreToolUse inside the tutor subagent only) | Blocks writes outside `docs/learning/` and any Bash other than read-only git |
| Git pre-commit | `Tools/githooks/pre-commit` | Same Content block as the commit guard, for commits made by hand |
| Git LFS hooks | `Tools/githooks/pre-push`, `post-checkout`, `post-commit`, `post-merge` | Git LFS's own hooks. They must live here because `core.hooksPath` replaces `.git/hooks`; without `pre-push`, pushes would send pointers without their LFS objects |

Set up a clone with `sh Tools/githooks/install.sh`.

## CI

`.github/workflows/repo-checks.yml` runs `.github/scripts/repo_checks.py` on every push and pull request. GitHub-hosted runners can't build Unreal, so CI only runs cheap checks:

- License boundary: no tracked file under `Content/` outside `Content/Takeover/`, `Content/__ExternalActors__/`, `Content/__ExternalObjects__/`. The folder list comes from `FIRST_PARTY` in `Tools/VendorContent/vendor_content.py`.
- `Tools/VendorContent/manifest.json` parses, and every entry has a path, a known source, a size, and a SHA-256.
- Every file matching an LFS pattern in `.gitattributes` is an LFS pointer in git.
- No blob over 1 MiB outside LFS, except `Tools/VendorContent/manifest.json`.
- Relative links in Markdown resolve (offline; external links are not checked).
- pytest for `.github/scripts/` and any `Tools/` tests.

Checkout uses `lfs: false`, so CI never downloads LFS objects and doesn't spend the 10 GiB a month of LFS bandwidth.

A self-hosted runner on the Windows machine is the later path to real build and test CI. Adding one is an ask-first change: it runs workflow code on the dev machine, and on a public repo that must never include workflows triggered from forks.

## GitHub setup

`.github/scripts/bootstrap_github.py` creates the labels, milestones, and M2 issues with `gh`, and can set the default branch and branch protection. Run it on a machine where `gh auth status` passes. It is idempotent.

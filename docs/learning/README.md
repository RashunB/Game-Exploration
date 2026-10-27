# Learning cycle

This repo has two products: the game, and Rashun's understanding of how it is built. Claude builds each feature to a high bar; this folder is where the "why" gets taught and tracked.

## The cycle

Each feature runs these steps:

1. Plan with reasons. A Superpowers spec and plan in `docs/superpowers/`, where every significant choice states what was chosen, the alternatives, why this one wins here, and what would make it wrong later. Rashun approves both.
2. Build, under the overrides in `CLAUDE.md`.
3. Verify: automation tests, build output, an MCP read-back, or a playtest for feel work (`docs/playtests/`).
4. Walkthrough. When the branch is ready to merge, `/walkthrough` explains the diff in reading order: data, then logic, then wiring. Rashun asks questions. He merges only after it.
5. Learning note. When the feature introduces a concept that has no note yet, `/learning-note` writes `notes/NNNN-<feature>.md` from `notes/TEMPLATE.md` and updates the concept map and glossary. Features that only reuse known concepts get the walkthrough and a concept map update, no note.
6. Your turn. Each milestone plan reserves one or two small, real tasks marked `owner: rashun` (label `owner:rashun` on GitHub). Rashun implements them and Claude reviews instead of writing. Good candidates touch the milestone's new concepts: a tuning field on a DataAsset, a new scoring state, a new heat event, a new test case.

At each milestone gate, `/retro` drafts `retros/<milestone>.md`: what was built, which concepts were introduced, what Rashun can now explain on his own, and what is still opaque. Rashun edits it.

## Files

| File | What it is |
| --- | --- |
| [concept-map.md](concept-map.md) | Every Unreal and game dev concept the repo uses, where it appears, and Rashun's status with it |
| [glossary.md](glossary.md) | Plain definitions, each tied to a file in this repo |
| [notes/TEMPLATE.md](notes/TEMPLATE.md) | Shape of a learning note |
| `notes/NNNN-<feature>.md` | One note per new concept, numbered in writing order |
| `retros/` | Milestone retros |
| [../playtests/TEMPLATE.md](../playtests/TEMPLATE.md) | Shape of a playtest log entry |

Concept status moves `not yet` → `seen` (it is in code Rashun has looked at) → `explained` (Rashun has had the walkthrough or read the note) → `can explain`. Only Rashun sets `can explain`.

## Commands

| Command | Runs in | Does |
| --- | --- | --- |
| `/walkthrough [range]` | this session | Explains a diff (default: the branch against `main`) in reading order, one stop at a time, and waits for questions. Moves covered concepts to `explained`. Runs in the main session because a subagent cannot hold a conversation. |
| `/learning-note [range]` | `tutor` subagent | Writes the next note for a feature or commit range, links it from the concept map, adds glossary terms |
| `/your-turn` | this session | Lists open `owner: rashun` tasks for the current milestone, with hints and no solutions. Reviews Rashun's attempt when he says it's ready |
| `/quiz [concept]` | this session | Asks questions from existing notes one at a time, and records weak spots in the concept map's notes column |
| `/retro` | `tutor` subagent | Drafts the milestone retro from git log, notes, and playtests for Rashun to edit |

The `tutor` subagent (`.claude/agents/tutor.md`) can read the repo and run read-only git, and can write only under `docs/learning/`. A hook enforces that.

## Output styles

Claude Code ships two built-in output styles that suit this cycle. Switch with `/output-style <name>`, and back with `/output-style default`.

- Explanatory adds short "Insight" blocks explaining the choices behind the code. Good for walkthrough and quiz sessions.
- Learning adds the same insights and leaves small `TODO(human)` pieces of code for you to write, then reviews them. Good for your-turn sessions. It writes those markers into the files, so use it only on your-turn branches.

Build sessions stay on the default style.

## Learning backlog for M0 and M1

Everything built before this cycle existed skipped the walkthrough, note, and your-turn steps. The notes below close that gap, in order from the most self-contained to the most engine-dependent. Each carries one your-turn task.

| Note | Covers | Status |
| --- | --- | --- |
| [0001-repo-skeleton](notes/0001-repo-skeleton.md) | `.uproject` and plugins, Build.cs, targets, module files, the smoke test, LFS layout, MCP config | written |
| [0002-scoring-model](notes/0002-scoring-model.md) | `FTakeoverScoringModel`, slip angle and yaw rate, state priority, combo, proximity, the headless spec | written |
| 0003-scoring-component-and-config | how the component samples Chaos each tick, collision filtering, crowd hits, `UTakeoverScoringConfig` | issue open |
| 0004-vehicle-pawn-and-handling | `ATakeoverVehiclePawn`, `UTakeoverHandlingProfile`, runtime vs wheel-class values, UE-274631, gearbox and damping fixes, telemetry | issue open |
| 0005-input-and-controller | Enhanced Input, `UTakeoverInputConfig`, context swapping, game mode wiring | issue open |
| 0006-arena-and-greybox | `ATakeoverArena` and its spline, `L_Greybox`, World Partition and One File Per Actor | issue open |
| 0007-vendor-content-pipeline | why third-party content stays out, manifest, dependency closure, restore, verify, record | issue open |

Write these before or alongside M2, not after.

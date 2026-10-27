# Takeover

Single-player crime/driving game in Unreal Engine 5.8, C++ first. The spec, architecture, milestones, and scope are in `PLAN.md`. Read it before starting a milestone. These rules apply to any coding agent working in this repo. Claude Code also reads `CLAUDE.md`.

The owner, Rashun, is the only developer. He has a CS degree and a strong Linux, git, and infrastructure automation background, and is new to game development, Unreal, and game-oriented C++. Every feature is built to a high bar and then explained to him (see Learning cycle). Skip git, shell, and general engineering basics. Do not assume knowledge of Unreal's object model, the gameplay framework, game math, physics tuning, or AI.

## Environment

- Building, the editor, Unreal MCP, and tests only work on the Windows dev machine with the agent running locally. A cloud session can edit text but cannot compile or test. Never claim code compiles or tests pass unless they were run in that session.
- Anything that needs the Windows machine goes into `docs/windows-checklist.md`.

## Commands

Run from the repo root in Git Bash on Windows. `UE_ROOT` is the engine install, for example `C:/Program Files/Epic Games/UE_5.8`.

Set up git hooks once per clone (pre-commit Content guard plus Git LFS's hooks):

```bash
sh Tools/githooks/install.sh
```

Restore third-party Content after a fresh clone, and verify it before committing level changes. Needs the staging project (`--staging`, default `C:/project_workspace/Unreal Engine/CitySample/Content`) and the engine install. Python 3.9+; the engine's own `Engine/Binaries/ThirdParty/Python3/Win64/python.exe` works:

```bash
python Tools/VendorContent/vendor_content.py restore
python Tools/VendorContent/vendor_content.py verify
```

Build the editor target (editor closed):

```bash
"$UE_ROOT/Engine/Build/BatchFiles/Build.bat" TakeoverEditor Win64 Development -Project="$(cygpath -w "$PWD/Takeover.uproject")" -WaitMutex
```

Run automation tests headless:

```bash
"$UE_ROOT/Engine/Binaries/Win64/UnrealEditor-Cmd.exe" "$(cygpath -w "$PWD/Takeover.uproject")" -ExecCmds="Automation RunTests Takeover; Quit" -TestExit="Automation Test Queue Empty" -unattended -nullrhi -nosplash -log
```

Run the repo checks CI runs (any OS, Python 3.9+):

```bash
python .github/scripts/repo_checks.py
```

## Working rules

- Start each milestone by restating its scope and acceptance criteria, then propose a task list before writing code.
- One feature per branch off `main`, in one checkout. No git worktrees: each one needs its own multi-gigabyte build output, LFS checkout, and full compile, and the editor and MCP server are bound to one project directory. Commit after every working step with a clear message.
- Changes reach `main` only through a pull request that uses `.github/pull_request_template.md`. Rashun merges, after the walkthrough.
- Never hand-edit binary `.uasset` or `.umap` files. Change them only through the editor or MCP. If an editor change goes wrong, revert the asset from git.
- Save and commit before any MCP session that touches several assets.
- Expose every tuning value as `UPROPERTY(EditDefaultsOnly)` or in a DataAsset so Rashun can tune without code changes.
- Never commit third-party content (City Sample, Fab, engine templates). The repo is public and their licenses don't allow it. `.gitignore` tracks only `Content/Takeover/` and the level's `__ExternalActors__`/`__ExternalObjects__`. Migrate only the assets in use with `Tools/VendorContent/migrate.py`, never edit them, and run `vendor_content.py record` so `manifest.json` lists them (see `PLAN.md` section 3). The pre-commit hook and CI both block a commit that breaks this.
- Ask Rashun before: adding any paid asset, changing engine version, adding plugins beyond those in `Takeover.uproject` or listed in `PLAN.md`, updating the agent tooling in `docs/tooling.md`, adding a self-hosted CI runner, large refactors, or anything touching Steam or store setup.
- Keep `CHANGELOG.md` current and update `PLAN.md` when scope changes. Log any AI-generated art, audio, or text that will ship, for the Steam content survey.
- Record significant decisions as ADRs in `docs/decisions/` (template there) and link them from the matching `PLAN.md` section.

## Testing and verification

- Test first for pure logic: scoring model, mission state transitions, heat tiers, decay and hysteresis, part values, failure reward math, save data round-trips. Unreal Automation tests, names starting with `Takeover.`, in `Source/Takeover/Tests/`, guarded by `WITH_DEV_AUTOMATION_TESTS`. Run them before marking a task done.
- `Tools/` Python follows the same rule with pytest. `Tools/VendorContent/vendor_content.py` guards the license boundary, so its logic (first-party vs third-party paths, hash checks, refusing edited files) gets tests before it changes again. Tests added to code that already exists pin down its current behavior first.
- This is why testable math lives in plain structs or functions with no `UWorld` dependency, and components only feed them state: the tests then run headless with `-nullrhi`.
- Not test-first: vehicle feel, camera, VFX, audio, AI difficulty, anything judged by playing. Do not write tests that only assert tuning constants. Feel work is verified by Rashun's playtest, logged in `docs/playtests/`. Do not mark feel-dependent tasks (handling, scoring balance, AI difficulty) done; hand them over for a playtest.
- Done means the evidence was observed in this session: build output, the automation test result, or an MCP read-back or screenshot. In a cloud session nothing that needs a build is done; it becomes a `docs/windows-checklist.md` item.

## Plans and specs

- Every spec and plan states, for each significant choice: what was chosen, the alternatives considered, why this one wins for this project (two to five sentences), and what would make the choice wrong later. The reasoning is learning material.
- Rashun approves the spec and the plan before execution starts. No autonomous run continues past an approval point.
- Each milestone plan reserves one or two small, real tasks for Rashun, marked `owner: rashun`: he implements them, the agent reviews. Pick tasks that touch the concepts the milestone introduced.

## Learning cycle

The repo's second product is Rashun's understanding of it. Each feature runs: plan with reasons, build, verify, walkthrough of the diff before merge, a learning note when the feature introduces a new concept, and his "your turn" tasks. Details, the concept map, and the note template are in `docs/learning/README.md`. Only Rashun moves a concept to `can explain` in `docs/learning/concept-map.md`.

## Code conventions

- Epic coding standard: tabs, PascalCase, `U`/`A`/`F`/`E`/`I`/`T` type prefixes, `b` prefix on bools.
- Include headers by path from the module root: `#include "Vehicle/TakeoverVehiclePawn.h"`.
- Keep testable math in plain structs or functions with no `UWorld` dependency. Components feed them state.
- Log with `UE_LOG(LogTakeover, ...)`.
- Single-player only: no replication, RPCs, authority checks, or GAS.

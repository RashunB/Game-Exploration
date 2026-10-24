# Takeover

Single-player crime/driving game in Unreal Engine 5.8, C++ first. The spec, architecture, milestones, and scope are in `PLAN.md`. Read it before starting a milestone.

## Environment

- Building, the editor, Unreal MCP, and tests only work on the Windows dev machine with Claude Code running locally. A cloud session can edit text but cannot compile or test. Never claim code compiles or tests pass unless they were run.
- Unreal MCP: `unreal-mcp` at http://127.0.0.1:8000/mcp, provided by the `unreal-engine-skills-for-claude-code` plugin. Setup and cautions are in `PLAN.md` section 3.

## Commands

Run from the repo root in Git Bash on Windows. `UE_ROOT` is the engine install, for example `C:/Program Files/Epic Games/UE_5.8`.

Restore third-party Content after a fresh clone, and verify it before committing level changes. Needs the staging project (`--staging`, default `C:/project_workspace/Unreal Engine/CitySample/Content`) and the engine install. Python 3.9+; the engine's own `Engine/Binaries/ThirdParty/Python3/Win64/python.exe` works:

```bash
python Tools/VendorContent/vendor_content.py restore
```

Build the editor target (editor closed):

```bash
"$UE_ROOT/Engine/Build/BatchFiles/Build.bat" TakeoverEditor Win64 Development -Project="$(cygpath -w "$PWD/Takeover.uproject")" -WaitMutex
```

Run automation tests headless:

```bash
"$UE_ROOT/Engine/Binaries/Win64/UnrealEditor-Cmd.exe" "$(cygpath -w "$PWD/Takeover.uproject")" -ExecCmds="Automation RunTests Takeover; Quit" -TestExit="Automation Test Queue Empty" -unattended -nullrhi -nosplash -log
```

With the editor open, compile through Live Coding (MCP `LiveCodingToolset.CompileLiveCoding`) instead of `Build.bat`. Live Coding does not handle class layout changes (new or removed `UPROPERTY`, `UFUNCTION`, `UCLASS`, `USTRUCT`) reliably. For those, close the editor and run a full build.

## Working rules

- Start each milestone by restating its scope and acceptance criteria, then propose a task list before writing code.
- One feature per branch. Commit after every working step with a clear message.
- Never hand-edit binary `.uasset` or `.umap` files. Change them only through the editor or MCP. If an editor change goes wrong, revert the asset from git.
- Save and commit before any MCP session that touches several assets.
- Expose every tuning value as `UPROPERTY(EditDefaultsOnly)` or in a DataAsset so the user can tune without code changes.
- Write automation tests for scoring, heat, state transitions, and part values. Run them before marking a task done.
- Do not mark feel-dependent tasks (handling, scoring balance, AI difficulty) done. Hand them to the user for a playtest.
- Never commit third-party content (City Sample, Fab, engine templates). The repo is public and their licenses don't allow it. `.gitignore` tracks only `Content/Takeover/` and the level's `__ExternalActors__`/`__ExternalObjects__`. Migrate only the assets in use with `Tools/VendorContent/migrate.py`, never edit them, and run `vendor_content.py record` so `manifest.json` lists them (see `PLAN.md` section 3).
- Ask the user before: adding any paid asset, changing engine version, adding plugins beyond those in `Takeover.uproject` or listed in `PLAN.md`, large refactors, or anything touching Steam or store setup.
- Keep `CHANGELOG.md` current and update `PLAN.md` when scope changes. Log any AI-generated art, audio, or text that will ship, for the Steam content survey.

## Code conventions

- Epic coding standard: tabs, PascalCase, `U`/`A`/`F`/`E`/`I`/`T` type prefixes, `b` prefix on bools.
- Include headers by path from the module root: `#include "Vehicle/TakeoverVehiclePawn.h"`.
- Keep testable math in plain structs or functions with no `UWorld` dependency. Components feed them state.
- Log with `UE_LOG(LogTakeover, ...)`.
- Automation test names start with `Takeover.` and live in `Source/Takeover/Tests/`, guarded by `WITH_DEV_AUTOMATION_TESTS`.

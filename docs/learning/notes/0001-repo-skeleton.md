# 0001: Repo skeleton

Branch or commits: `fddf8cf` (M0 skeleton), plus `9667140` (LiveCodingToolset) · Written: 2026-10-27 · Concept map rows: `.uproject` file and project plugins, Modules and `Build.cs` dependencies, Build targets, Unreal Build Tool, Live Coding vs full builds, Log categories, Automation tests and specs, Headless test runs, Git LFS, Unreal MCP editor control

## What this feature does

It turns an empty folder into something Unreal can open, compile, and test. In code terms: a project manifest names one C++ module and the engine plugins it needs, two build targets say what to compile it into (the editor or the shipping game), the module's own build file lists the engine modules it links against, and one smoke test proves the whole chain works headless.

## Files to read, in order

1. `Takeover.uproject` (whole file, 65 lines): the project manifest. Data first.
2. `Source/Takeover.Target.cs` and `Source/TakeoverEditor.Target.cs` (12 lines each): the two targets.
3. `Source/Takeover/Takeover.Build.cs` (23 lines): the module's build rules.
4. `Source/Takeover/Takeover.h` and `Source/Takeover/Takeover.cpp` (5 and 6 lines): the module itself and its log category.
5. `Source/Takeover/Tests/TakeoverSmokeTest.cpp` (16 lines): the proof it all links and loads.
6. Around the edges: `.gitattributes`, `.gitignore` (lines 1 to 7), `.mcp.json`, `Config/DefaultEditorPerProjectUserSettings.ini`.

## The core idea

Unreal's build is a three-level graph: project, targets, modules. It is closer to a monorepo build tool (Bazel, or a Makefile with per-directory rules) than to an IDE project, and the Visual Studio solution is generated from it, not the other way round.

The project is `Takeover.uproject`. It is JSON, and the editor reads it to decide what to load. `EngineAssociation` (`Takeover.uproject:3`) pins the engine version, `Modules` (`Takeover.uproject:6`) lists the C++ modules this project owns (one, `Takeover`, with `"Type": "Runtime"` so it is part of the shipped game), and `Plugins` (`Takeover.uproject:16`) switches engine plugins on. A plugin is a packaged bundle of modules and content that ships with the engine but is off until a project enables it, like an optional package a config management state pulls in. `ChaosVehiclesPlugin` and `EnhancedInput` are gameplay plugins. The three MCP plugins and `ModelingToolsEditorMode` carry `"TargetAllowList": ["Editor"]` (`Takeover.uproject:36`, `:43`, `:50`, `:57`), so they exist only in the editor build and can never ship in the game.

A target is one thing Unreal Build Tool (UBT) can produce. `TakeoverEditor.Target.cs` builds `TargetType.Editor` (the project's modules compiled as DLLs the editor loads) and `Takeover.Target.cs` builds `TargetType.Game` (the standalone executable). Both are C# classes that UBT compiles and instantiates to read their settings: `ExtraModuleNames.Add("Takeover")` says which modules to build, and `BuildSettingsVersion.Latest` / `EngineIncludeOrderVersion.Latest` opt into the newest default compiler settings and include order for this engine version. The Windows command in `AGENTS.md` builds the `TakeoverEditor` target, which is why it names that target and not the project.

A module is a unit of compiled C++ with its own `Build.cs`, roughly a shared library with a dependency manifest. `Takeover.Build.cs:12` lists `PublicDependencyModuleNames`: engine modules whose headers this module includes and whose symbols it links against. Miss one and the build fails with a missing include or an unresolved external symbol. `EnhancedInput` and `ChaosVehicles` are there because the pawn and controller include their headers (`Source/Takeover/Vehicle/TakeoverVehiclePawn.cpp:8`, `:9`). Public dependencies are passed on to any module that depends on this one; private ones (none yet) are not. `PublicIncludePaths.Add(ModuleDirectory)` (`Takeover.Build.cs:10`) is what lets every file write `#include "Vehicle/TakeoverVehiclePawn.h"`, a path from the module root, as `AGENTS.md` requires.

The module's code starts in `Source/Takeover/Takeover.cpp:4`: `IMPLEMENT_PRIMARY_GAME_MODULE` registers `Takeover` as the project's main game module, with the stock `FDefaultGameModuleImpl` because nothing needs to run at module startup. Line 6 defines the `LogTakeover` category that `Takeover.h:5` declares, so every file can log with `UE_LOG(LogTakeover, ...)` and the output can be filtered by category.

The smoke test closes the loop. `IMPLEMENT_SIMPLE_AUTOMATION_TEST` (`Source/Takeover/Tests/TakeoverSmokeTest.cpp:7`) registers a test named `Takeover.Smoke.ModuleLoaded`, and the body asserts that the module manager has the `Takeover` module loaded. Passing it proves that the targets build, the module loads, test discovery finds tests by their dotted name, and the headless command (`UnrealEditor-Cmd.exe ... -ExecCmds="Automation RunTests Takeover; Quit" -nullrhi` in `AGENTS.md`) works without a GPU. Everything is inside `#if WITH_DEV_AUTOMATION_TESTS`, so tests are compiled out of shipping builds.

Around the edges:

- Git LFS. `.gitattributes:12` and `:13` route every `.uasset` and `.umap` through LFS: git stores a small pointer and the bytes live on the LFS server. Binary assets don't diff or merge, and every re-save stores a full new copy, which is why the 10 GiB LFS budget in `PLAN.md` section 3 matters.
- What is tracked. `.gitignore:4` to `:7` ignore everything under `Content/` except the project's own folders. That is the license boundary (0007 covers it).
- Editor control. `.mcp.json` points Claude Code at the Unreal MCP server inside the running editor, and `Config/DefaultEditorPerProjectUserSettings.ini` starts that server with the editor. Because the server lives in the editor process, a full build (editor closed) drops the connection, while Live Coding patches function bodies into the running editor and keeps it. Live Coding can't change a class's memory layout, which is why adding a `UPROPERTY` needs a full build (`CLAUDE.md`, Unreal MCP).

## Why it is built this way

- One module, not several. A single `Takeover` runtime module is the default for a small game. Splitting out modules pays off when code must load at different times or be shared across projects; neither applies yet. `PLAN.md` section 2 fixes the code name `Takeover` because renaming a module later needs CoreRedirects for every Blueprint subclass ([ADR 0002](../../decisions/0002-cpp-first-thin-blueprints.md) covers the C++-first choice).
- Feature folders in one module (`Arena/`, `Vehicle/`, `Player/`) instead of Unreal's `Public/`/`Private/` split. The split controls which headers other modules can include. With one module there are no other modules, so the split would add a directory level and buy nothing.
- MCP plugins limited to the Editor target. The server has no authentication and can run arbitrary Python in the editor (`PLAN.md` section 3), so it must never ship. M6 still checks a packaged build for MCP code.
- `LiveCodingToolset` enabled separately (`9667140`): `AllToolsets` does not include it, and without it Live Coding can't run over MCP ([ADR 0013](../../decisions/0013-enable-livecodingtoolset.md)).

## What would break if you changed X

1. Remove `"EnhancedInput"` from `PublicDependencyModuleNames` in `Takeover.Build.cs` and run the editor build from `AGENTS.md`. Expect it to fail in `TakeoverVehiclePawn.cpp` or `TakeoverPlayerController.cpp`, either on the `EnhancedInputComponent.h` include or at link time. Read which: it tells you whether the missing piece was the include path or the library. Revert.
2. Rename the smoke test to `Smoke.ModuleLoaded` (drop the `Takeover.` prefix), rebuild, and run the headless test command. Expect it to vanish from the run, because `RunTests Takeover` selects tests by name prefix. That is why `AGENTS.md` requires the prefix. Revert.
3. Change `TEXT("Takeover")` to `TEXT("Takeover2")` inside `IsModuleLoaded` in the smoke test and run the tests. Expect one failure with the message "Takeover module is loaded" in the log and a non-zero exit. This is what a failing test looks like from the command line. Revert.

## Check yourself

1. Why do the MCP plugins carry `TargetAllowList: ["Editor"]`, and what would go wrong without it?
2. A new file includes `"Arena/TakeoverArena.h"`. What line makes that path resolve?
3. Why does a full build drop the Unreal MCP connection when Live Coding doesn't?
4. What is the difference between the `TakeoverEditor` and `Takeover` targets, and which one does the build command in `AGENTS.md` build?
5. What does a `.uasset` look like inside git, and what does CI check about it?

<details>
<summary>Answers</summary>

1. They are editor tooling with no authentication and the ability to run arbitrary Python (`PLAN.md` section 3). Without the allow list they would also compile into the Game target and could ship to players. `Takeover.uproject:43`.
2. `PublicIncludePaths.Add(ModuleDirectory)` at `Source/Takeover/Takeover.Build.cs:10` adds the module root to the include path.
3. The MCP server runs inside the editor process. A full build needs the editor closed, because the editor holds the module DLLs, so the server goes away. Live Coding patches new function bodies into the running editor, so the process and the server stay up. `CLAUDE.md` (Unreal MCP).
4. `TakeoverEditor` (`Source/TakeoverEditor.Target.cs:7`) builds the module as DLLs for the editor; `Takeover` (`Source/Takeover.Target.cs:7`) builds the standalone game. `AGENTS.md` builds `TakeoverEditor`.
5. A short text pointer (`version https://git-lfs.github.com/spec/v1`, an oid, a size); the bytes live on the LFS server. `.gitattributes:12`. CI's `lfs` check in `.github/scripts/repo_checks.py` fails if a file matching an LFS pattern was committed as raw bytes instead.

</details>

## Your turn

Add an automation test `Takeover.Smoke.PluginsEnabled` that fails if `ChaosVehiclesPlugin` or `EnhancedInput` is not enabled. It protects against someone switching a gameplay plugin off in the `.uproject`.

- Files: a new test in `Source/Takeover/Tests/`, and `Source/Takeover/Takeover.Build.cs`.
- Done when: the test passes in the headless run, and fails if you temporarily set `"Enabled": false` on `EnhancedInput` in `Takeover.uproject`.
- Hints: the engine's plugin registry is `IPluginManager` in the `Projects` module, header `Interfaces/IPluginManager.h`. Your test needs that module as a dependency; decide whether it belongs in the public or private list, and why. Copy the shape of `TakeoverSmokeTest.cpp`.

## Further reading

Confirmed 2026-10-27 through search results; Epic's site was not reachable from the cloud session that wrote this note.

- [Unreal Engine Modules](https://dev.epicgames.com/documentation/en-us/unreal-engine/unreal-engine-modules)
- [Unreal Build Tool target reference](https://dev.epicgames.com/documentation/unreal-engine/unreal-engine-build-tool-target-reference)
- [Automation Spec in Unreal Engine](https://dev.epicgames.com/documentation/en-us/unreal-engine/automation-spec-in-unreal-engine)

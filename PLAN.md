# Takeover (working title): Development Plan

Handoff document for coding agents. Read this fully before starting any work. It defines the game, the architecture, the build order, and the scope. The working rules live in `AGENTS.md` (any agent) and `CLAUDE.md` (Claude Code only, imports `AGENTS.md`), so they load automatically.

## 1. Concept

A single-player, third-person crime/driving game built around one mission loop, inspired by GTA heist missions and Need for Speed drift events. The tone reference is short-clip, stream-friendly chaos (Hood Warfare 2 got attention for being funny in clips). Readability and spectacle beat realism.

The loop has five phases, played in order:

1. Break-in: sneak into a house and find the car keys.
2. Getaway: steal the car and escape with heat (wanted level) building.
3. Takeover: drive to a street takeover, score points for donuts, drifts, and burnouts in front of a crowd, then complete a drift or race event.
4. Chop: take the car to a chop shop and part it out for money.
5. Dump: ditch the stripped shell at a drop zone without getting caught.

Money and rep carry over between loops. Heat carries across phases within a loop and resets when the loop ends.

## 2. Hard constraints

- Engine: Unreal Engine 5.8 (launcher build, 5.8.4 at time of writing), Chaos Vehicles, Enhanced Input. Moved from 5.7 because the Unreal MCP editor plugin that Epic's Claude Code plugin depends on ships with 5.8. 5.8 is expected to be the last 5.x release, so no engine upgrade is planned before Early Access. ([ADR 0001](docs/decisions/0001-ue-5-8-over-5-7.md))
- Platform for Early Access: Windows only, keyboard/mouse and gamepad.
- C++ first. All gameplay logic lives in C++. Blueprints are thin subclasses that hold tuning values and asset references only. Reason: Claude can read, diff, and refactor C++ as text. Blueprints are binary .uasset files and can only be changed through the editor. ([ADR 0002](docs/decisions/0002-cpp-first-thin-blueprints.md))
- Free assets by default (see section 7). No paid asset gets added without asking the user first.
- All cars, brands, and logos are fictional. No real manufacturer names, badges, or recognizable body designs. ([ADR 0008](docs/decisions/0008-fictional-cars-and-brands.md))
- Single-player only for Early Access. Do not add networking or replication.
- The code name `Takeover` is fixed for the module, the `.uproject`, and class names, whatever the store title ends up being. Renaming a module later needs CoreRedirects for every Blueprint subclass.

## 3. Development environment

### Machines

The Windows dev machine is the only place that builds, runs the editor, and runs tests. It needs:

- UE 5.8.x from the Epic Games Launcher.
- Visual Studio 2022 17.14 or later with the "Game development with C++" workload, MSVC 14.38 or later (Epic's 5.8 build farm uses 14.44), and Windows SDK 10.0.22621.0. VS 2026 (18.0+) is supported, but 5.8.1 reports show project generation still looking for VS 2022, so keep 2022 installed.
- Git for Windows (Git Bash is required by the Epic plugin's SessionStart hook) and Git LFS.
- Claude Code running locally from the repo root. The Unreal MCP server binds to 127.0.0.1, so only a session on the same machine can reach the editor.

Cloud Claude Code sessions (claude.ai/code) can edit C++, config, and docs, but cannot compile, run tests, or reach the editor. Anything written in a cloud session gets built and tested on the Windows machine before it merges.

### Version control

- git with Git LFS. `.gitattributes` routes `*.uasset`, `*.umap`, and binary media through LFS. `.gitignore` excludes Binaries, Intermediate, Saved, DerivedDataCache, packaging output, IDE files, and generated solution files.
- No LFS file locking. This is a solo project, and lockable files stay read-only until locked, which fights the editor's auto-save. ([ADR 0007](docs/decisions/0007-git-lfs-without-locking.md))
- LFS budget: GitHub Free includes 10 GiB storage and 10 GiB bandwidth per month. Overage is metered ($0.07/GiB storage, $0.0875/GiB bandwidth). Without a payment method on file, pushes of new LFS objects stop once storage is exceeded. Every re-save of a binary asset stores a full new copy. Only first-party assets go through LFS (third-party content is not committed), so the budget covers this project's own maps, Blueprints, and art.
- Third-party content stays out of git. The repo is public, and City Sample and Fab content is licensed for use in Unreal Engine products, not for publishing as source files. Add packs to the local staging project (`C:/project_workspace/Unreal Engine/CitySample`). Copy only the assets in use, with their dependency closure, using `Tools/VendorContent/dep_closure.py` and `migrate.py`. Then run `vendor_content.py record`. `Tools/VendorContent/manifest.json` (committed) lists every third-party file with its source and SHA-256, and `vendor_content.py restore` rebuilds them on a fresh clone. `.gitignore` tracks only `Content/Takeover/`, `Content/__ExternalActors__/`, and `Content/__ExternalObjects__/`. Third-party files are never edited, so the manifest hashes stay valid. The engine's vehicle template restores from the engine install. `Tools/githooks/pre-commit` and CI (`.github/workflows/repo-checks.yml`) both reject a commit that tracks anything under `Content/` outside those three folders. ([ADR 0009](docs/decisions/0009-third-party-content-out-of-repo.md))
- Backups: the staging project and the Fab library are the only copies of third-party content outside this machine. Keep the staging project backed up. Restoring needs the same pack versions, and `restore` refuses a source file whose hash changed.
- Maps use World Partition with One File Per Actor, so editing a level changes small per-actor files instead of rewriting one large `.umap`. ([ADR 0003](docs/decisions/0003-one-world-partition-map.md))

### Editor access (Unreal MCP)

- Engine side: the `ModelContextProtocol`, `AllToolsets`, and `LiveCodingToolset` plugins, all Experimental, enabled in the `.uproject` for the Editor target only. `ModelContextProtocol` is the server; `AllToolsets` provides the tools, and the server exposes none without it. `LiveCodingToolset` is not part of `AllToolsets` and is off by default, so it is enabled separately for `LiveCodingToolset.CompileLiveCoding`. ([ADR 0013](docs/decisions/0013-enable-livecodingtoolset.md)) The launcher build ships no MCP proxy, so a full build (editor closed) drops the MCP connection; Live Coding keeps it for function-body changes. `Config/DefaultEditorPerProjectUserSettings.ini` sets `bAutoStartServer=True`, so the server starts with the editor on http://127.0.0.1:8000/mcp. Manual start: `ModelContextProtocol.StartServer` in the editor console.
- Claude side: Epic's plugin (`/plugin install unreal-engine-skills-for-claude-code@claude-plugins-official`), also enabled for this repo in `.claude/settings.json`. `.mcp.json` at the repo root points Claude Code at the server. If the port changes, regenerate it with `ModelContextProtocol.GenerateClientConfig ClaudeCode`.
- Use MCP for placing actors, creating Blueprint subclasses and DataAssets, Input Actions and Mapping Contexts, assigning assets, and Live Coding compiles. Use plain file edits for C++ and config.
- Cautions from Epic's README: the server has no authentication, so any local process can drive the editor while it is open. `ProgrammaticToolset.execute_tool_script` runs arbitrary Python with full editor access. Do not run Claude Code with `--dangerously-skip-permissions` while the plugin is loaded. Save and commit before long MCP sessions.

### Agent tooling and repo checks

- Three layers with one job each: Epic's plugin for editor control, 26 vendored `quodsoler/unreal-engine-skills` for Unreal C++ patterns, and the Superpowers plugin for process. Versions, review findings, hooks, CI, and the update procedure are in `docs/tooling.md`. Precedence and the Superpowers overrides (no git worktrees, TDD only for pure logic) are in `CLAUDE.md`. ([ADR 0014](docs/decisions/0014-agent-plugin-and-skill-stack.md))
- `AGENTS.md` holds the rules for any coding agent. `CLAUDE.md` imports it and adds Claude-specific rules.
- `main` is the default branch. One branch per feature, merged by the user through a pull request after a walkthrough.
- CI runs only cheap checks on GitHub-hosted runners (license boundary, manifest, LFS pointers, file size, links). Build and test CI needs a self-hosted runner on the Windows machine, which is ask-first.
- Each feature also runs the learning cycle in `docs/learning/README.md`: a walkthrough before merge, a learning note when it introduces a new concept, and one or two `owner: rashun` tasks per milestone.

### Testing

- Unreal Automation tests for pure logic: scoring math, state machine transitions, heat tiers and decay, part values. Test names start with `Takeover.`.
- Keep the math in plain structs and functions with no `UWorld` dependency, so tests run headless with `-nullrhi`.
- `Tools/` Python is tested with pytest, and CI runs those tests (`docs/tooling.md`).
- Feel work (handling, timing, balance, AI difficulty) is verified by the user's playtest, logged in `docs/playtests/`.
- Build and test commands are in `AGENTS.md`.

### Folder layout

```
AGENTS.md        rules for any coding agent
CLAUDE.md        Claude Code rules, imports AGENTS.md
PLAN.md
CHANGELOG.md
Takeover.uproject
.agents/         project context read by the quodsoler skills
.claude/         settings, hooks, agents, skills (vendored ue-* and the learning commands)
.github/         CI workflow, repo checks, issue and PR templates
docs/
  tooling.md     plugin and skill versions, hooks, CI, update procedure
  decisions/     ADRs
  learning/      learning cycle, concept map, glossary, notes, retros
  playtests/     feel-gate sign-offs
  superpowers/   specs and plans
  windows-checklist.md
Tools/
  VendorContent/ third-party content manifest and restore tooling
  githooks/      pre-commit Content guard and Git LFS hooks
Config/
Source/
  Takeover.Target.cs
  TakeoverEditor.Target.cs
  Takeover/
    Core/        GameMode, GameInstance, save game, progress subsystem
    Mission/     mission subsystem, phase classes, loop state, heat
    Player/      player controller, on-foot character, input config
    Vehicle/     vehicle pawn, handling profiles
    Arena/       takeover scoring, arena, crowd reactions
    Stealth/     interaction, noise, homeowner AI
    Pursuit/     police AI
    Chop/        part data, chop UI logic
    UI/          HUD widgets (C++ base classes)
    Tests/       automation tests
Content/
  Takeover/
    Blueprints/  thin BP subclasses only
    Data/        DataTables, DataAssets, curves
    Environment/ greybox city materials and meshes (placeholders until the art pass)
    Input/       Input Actions and Mapping Contexts
    Maps/        greybox and final maps
    UI/          widget Blueprints
  <PackName>/    migrated third-party content at its original path, never edited, not in git (Tools/VendorContent restores it)
```

Headers are included by path from the module root, for example `#include "Vehicle/TakeoverVehiclePawn.h"`.

## 4. Architecture

### World and lifetime

- One persistent World Partition map holds the whole loop: house, streets, arena, chop shop, drop zones. No level travel during a loop, so world-scoped systems keep their state across phases. ([ADR 0003](docs/decisions/0003-one-world-partition-map.md))
- `UTakeoverProgressSubsystem` (UGameInstanceSubsystem) owns money, rep, and loops completed, and loads and saves `UTakeoverSaveGame`.

### Mission spine

- `UMissionSubsystem` (UTickableWorldSubsystem, so phases tick without a host actor) owns the current loop and drives phase transitions. ([ADR 0004](docs/decisions/0004-mission-subsystem-with-phase-objects.md))
- `EMissionPhase`: Idle, BreakIn, Getaway, Takeover, Chop, Dump, Complete, Failed.
- Each phase is a `UMissionPhase` subclass with `Enter()`, `Tick()`, `Exit()`, and `OnPhaseComplete` / `OnPhaseFailed` delegates. Phases do not reference each other. The subsystem wires them together.
- Phase order and phase classes come from a `UMissionConfig` DataAsset.
- `FMissionLoopState`, owned by the subsystem and handed to each phase on `Enter()`, carries what phases need from each other: the stolen vehicle, takeover score and rep earned, chop payout, and failure reason.
- Failure (arrested, car destroyed) ends the loop with partial rewards: rep already banked from the takeover is kept, and unbanked money is scaled by `FailureRewardFraction` on `UMissionConfig` (default 0.25). ([ADR 0012](docs/decisions/0012-failure-rewards.md))

### Player and input

- `ATakeoverPlayerController` owns the heat component and swaps possession between the on-foot character and the vehicle. Heat lives here so it survives pawn swaps. ([ADR 0005](docs/decisions/0005-heat-on-player-controller.md))
- Enhanced Input mapping contexts: `IMC_OnFoot`, `IMC_Vehicle`, `IMC_Menu`. The controller swaps contexts on possession.
- Every action gets keyboard/mouse and gamepad bindings from M0. Drift feel gets tuned on an analog stick, so gamepad can't wait for the polish milestone.
- Input Actions and Mapping Contexts are assets created through the editor or MCP. C++ reaches them through a `UTakeoverInputConfig` DataAsset.

### Heat

- `UHeatComponent` on the player controller. Float 0 to 100 with tiers (0 clean, 1 noticed, 2 pursued, 3 raid). Tier thresholds and a hysteresis margin live in a DataAsset so the tier doesn't flicker at a boundary.
- Events add heat (alarm, sighting, crash into police, takeover duration). Heat decays when no police unit or camera has line of sight. Until pursuit AI exists (M4), the line-of-sight query is a stub that reports "unseen".
- Heat tier drives police spawn density and the takeover raid timer.

### Vehicle

- `ATakeoverVehiclePawn` subclasses `AWheeledVehiclePawn` with `UChaosWheeledVehicleMovementComponent`.
- Handling profiles as DataAssets: Street (getaway), Drift (takeover), Stripped (dump, on rims, degraded grip and power). Swap profiles at runtime by phase.
- Chaos exposes runtime setters for some values (torque, friction multipliers, drag), but wheel setup lives on wheel class defaults. In M1, confirm which profile fields apply live. Fields that don't get applied by recreating the vehicle's physics state at a point where the car is slow or stopped: the arena entry trigger and the chop shop.
- The hero car is a BP subclass of `ATakeoverVehiclePawn` that reuses a City Sample vehicle's mesh, physics asset, and wheel animation. It does not reuse the City Sample vehicle Blueprint.
- Give the hero car its own wheel classes, not shared with police or traffic cars. Epic issue UE-274631 reports per-car wheel configuration writing to the shared wheel class defaults, which would leak handling changes between cars. ([ADR 0010](docs/decisions/0010-hero-car-own-wheel-classes.md))
- Expect real tuning time. Drift feel is iteration with the user, not something to mark done from code alone.

### Persistence

- `UTakeoverSaveGame`: money, rep, loops completed, settings. Save on loop end and on quit.

## 5. Phase specs

Each phase lists the Early Access scope and acceptance criteria. Anything not listed is out of scope for EA.

### 5.1 Break-in

- Third-person character based on the UE 5.8 C++ Third Person template, brought into `Player/` as `ATakeoverCharacter`.
- `UInteractableComponent` for doors, drawers, key hooks, with a hold-to-interact prompt.
- Noise system: sprinting and knocking objects over emit noise events with a radius.
- One NPC (homeowner) on StateTree (`UStateTreeAIComponent` from the GameplayStateTree plugin) with AI Perception (sight and hearing). States: Patrol, Investigate, Alert. Alert triggers the alarm and adds heat.
- Keys spawn at one of N marked locations, chosen at random.
- Acceptance: player can sneak in, find keys, leave. Getting spotted raises heat and does not hard-fail the mission.

### 5.2 Getaway

- Enter the car with the keys, ignition, drive. Entering swaps possession and input context from on-foot to vehicle.
- Police pursuit AI follows the road spline network toward the player's predicted position and rams when close. Boxing the player in is a stretch goal. Spawn rate scales with heat tier.
- Police driving AI on Chaos is the largest unknown in this plan. Budget for it accordingly (see M4).
- Heat decays out of line of sight. Losing all pursuers is required before the takeover marker activates.
- Acceptance: a pursuit can be escaped, and arrest triggers when the car is stopped and surrounded for a set time.

### 5.3 Takeover (core loop, build first)

- Arena: one intersection or parking lot with a crowd ring defined by a spline.
- `UTakeoverScoringComponent` on the vehicle feeds vehicle state each tick into `FTakeoverScoringModel`, a plain struct with no world access, so the scoring math is covered by automation tests. ([ADR 0006](docs/decisions/0006-scoring-math-in-plain-struct.md)) All thresholds live on a `UTakeoverScoringConfig` DataAsset. Tunable defaults:
  - Slip angle is the angle between the car's forward vector and its velocity in the ground plane. It only counts above a minimum speed (default 20 km/h) so low-speed jitter doesn't score. Above 100 degrees the car is spinning out or reversing, which is not a drift.
  - Each tick the car is in exactly one scoring state, checked in priority order: Donut, Drift, Burnout, None. A donut also has a high slip angle, and the priority keeps it from scoring twice.
  - Drift points per second = (slip angle above 15 degrees) x speed factor x proximity multiplier. Speed factor is speed / 60 km/h, capped at 3.
  - Proximity multiplier rises from 1 at 15 m from the crowd spline to 3 at 2 m. It applies to drifts, donuts, and burnouts, since performing close to the crowd is the point of a takeover.
  - Combo multiplier increments every 2 seconds of continuous scoring (any state other than None), breaks on collision or after 1.5 seconds in None, capped at x10. All points are multiplied by it.
  - Donut detection: yaw rate above 120 deg/s, speed at most 30 km/h, and throttle held (so a spin-out after a crash doesn't score). Flat 40 points per second.
  - Burnout: throttle plus brake at low speed, scored by duration. Flat 20 points per second.
  - Collisions below an impulse threshold, and ground contact, don't break the combo.
  - Hitting a crowd member ends the combo and adds heat (heat hooks into the scoring component's crowd hit event in M2).
- Raid timer: when heat hits tier 3 during the takeover, police arrive and the player must leave. Leaving with a high score banks rep. Needs heat, so it lands in M2, not M1.
- Event: one drift event (score attack along a marked line) or one sprint race against simple AI. Chosen at the M1 gate: drift event. It reuses the scoring model, while a race needs AI drivers on Chaos, the same hard problem as police AI. ([ADR 0011](docs/decisions/0011-drift-event-finale.md))
- Feedback: score popups, combo meter, crowd cheer audio scaled to combo, phone-flash particles from the crowd, tire smoke and skid marks. M1 only needs a debug score/combo readout, tire smoke, and skid marks. The crowd is placeholder meshes on the spline until the art pass.
- Acceptance: the user playtests the greybox on gamepad and keyboard and confirms it is fun. This is a gate. Do not move past milestone M1 without that confirmation.

### 5.4 Chop

- EA scope is menu-based. Arrive at the garage, open the chop UI, remove parts one by one.
- Parts defined in a DataTable (`FChopPartRow`): id, display name, base value, removal time, and an optional mesh to hide on removal.
- Removal is a short timing minigame (hit the window to remove cleanly, miss to reduce value).
- Visually hide 2 or 3 parts on the car (wheels, hood, seats) where the free car mesh allows. Do not split meshes in Blender for EA.
- Acceptance: money is awarded per part, part values are covered by automation tests, and the car switches to the Stripped profile.

### 5.5 Dump

- Drive the stripped shell to one of several drop zones while patrol and camera detection cones are active.
- Reuses getaway pursuit and heat systems.
- Acceptance: dropping the shell undetected completes the loop. Getting caught fails it with partial rewards.

## 6. Steam

Steam tasks run in parallel with development. The user owns all of them.

- Pay the $100 Steam Direct fee in M0 or M1. First-time developers wait 30 days after paying before they can release, and bank, tax, and identity verification run in that window. Paying early only starts the clock sooner.
- The Coming Soon page must be public for at least two weeks before release. Store page and build reviews take 1 to 5 days each. For a release at the end of week 13, submit the store page for review by week 9 and have it public by week 10.
- AI disclosure (content survey, revised January 2026): code written with Claude does not need disclosure. Art, audio, or text generated by AI that ships in the game does. Log any such content in `CHANGELOG.md` as it is added so the survey is accurate.

## 7. Assets

Free (default):

- City Sample Vehicles (Fab): 13 driveable vehicles. Use as the hero car base and traffic/police placeholders. In M0, confirm the pack can be added to a 5.8 project. If Fab only offers older versions, add it to a staging project on a listed version and Migrate forward.
- City Sample Crowd (Fab): MetaHuman crowd for the takeover ring. Heavy. Placeholders until the art pass.
- City Sample Buildings (Fab): building kit for a small custom map. Do not build inside the full City Sample project (updated for 5.8 in August 2026), it is too heavy as a base.
- UE 5.8 C++ Third Person and Vehicle templates.
- Free Fab/Quixel materials and props for the house interior and garage.

Optional paid, ask the user first:

- Easy Vehicle System (Fab): enter/exit, lights, engine audio for Chaos. The listing names 5.6 and 5.7 only. Confirm 5.8 support with the seller before buying.
- Drift AI (Fab): drift AI with solo, tandem, and drift race modes. Its listings show 5.2 to 5.4. Treat it as incompatible until the seller confirms 5.8. Only relevant if the finale becomes a race.

City Sample content is licensed for Unreal Engine products only, which is fine for this project. That license, like Fab's, does not allow publishing the source assets, so third-party content stays out of the public repo (section 3).

## 8. Milestones (Early Access path)

Estimates assume one developer full-time with Claude doing most of the code. Part-time is roughly 3x.

1. M0, week 1. Setup.
   - Done in repo: `.uproject`, module skeleton, target files, folder layout, git and LFS config, MCP config, `CLAUDE.md`, `CHANGELOG.md`, a smoke automation test.
   - On the Windows machine: install the toolchain, run `git lfs install`, build the editor target, open the editor and confirm MCP responds (`/mcp` shows `unreal-mcp`, `list_toolsets` returns), create Input Actions and Mapping Contexts with keyboard/mouse and gamepad bindings, create the greybox lot map (World Partition, One File Per Actor), confirm City Sample Vehicles on 5.8, run the smoke test from the command line.
   - Acceptance: a fresh clone builds, the editor opens, MCP responds to a read-only tool call, `Takeover.Smoke` passes from the command line, and the greybox map loads.
2. M1, weeks 2 to 4. Vehicle pawn with the Drift profile, scoring model, component, and tests, placeholder crowd spline, debug feedback. Gate: user confirms it is fun.
   - Code (first pass, compiles clean on 5.8.4, `Takeover.*` tests pass; not yet driven in the editor): `ATakeoverGameMode`, `ATakeoverPlayerController`, `UTakeoverInputConfig`, `ATakeoverVehiclePawn`, `UTakeoverHandlingProfile`, `FTakeoverScoringModel`, `UTakeoverScoringConfig`, `UTakeoverScoringComponent`, `ATakeoverArena`, `ATakeoverDebugHUD`, `Takeover.Scoring.Model` tests.
   - Editor side, in `Content/Takeover/`:
     - `Input/`: `IA_Throttle`, `IA_Brake`, `IA_Steer` (Axis1D), `IA_Handbrake`, `IA_ResetVehicle` (Digital), `IA_LookAround` (Axis1D), and `IMC_Vehicle` binding all of them to keyboard/mouse and gamepad.
     - `Data/`: `DA_InputConfig` (UTakeoverInputConfig), `DA_Handling_Drift` (UTakeoverHandlingProfile), `DA_Scoring` (UTakeoverScoringConfig).
     - `Blueprints/`: `BP_HeroCar` (ATakeoverVehiclePawn: mesh, wheel classes, `DA_Handling_Drift`, `DA_Scoring` on the Scoring component), `BP_Arena` (ATakeoverArena: placeholder mesh such as the engine cylinder), `BP_TakeoverPlayerController` (`DA_InputConfig`), `BP_TakeoverGameMode` (default pawn `BP_HeroCar`, controller `BP_TakeoverPlayerController`).
     - `Maps/`: greybox lot with `BP_Arena` placed and `BP_TakeoverGameMode` as the world's game mode.
     - Tire smoke and skid marks (Niagara) after the car drives.
   - Editor side status (2026-10-24): all of the above except Niagara exists and spawns cleanly in PIE. `BP_HeroCar` uses the vehicle template's SportsCar as a stand-in mesh until City Sample Vehicles is migrated. Handbrake is bound to both Gamepad A and RB until the playtest picks one. `L_Greybox` uses a flat ground slab instead of the template landscape, which did not survive duplication. The crowd ring is a 120 x 70 m oval so there is a straight long enough to leave 1st gear. The lot is dressed as a night takeover intersection: two crossing avenues, 16 blocks of procedural-window towers, sidewalks, lane markings, 40 street lamps, 6 shadow-casting plaza floodlights, neon at street level, fixed night exposure. All engine shapes and authored materials, no Fab content yet. Handling is mid-tuning with telemetry (`takeover.Telemetry`).
   - Not yet built: a physical burnout. Scoring detects throttle plus brake, but with default wheel setup Chaos brakes every wheel, so the rears probably won't spin. Check in the first drive, and if so, brake only the front axle while both are held.
   - Gate passed (2026-10-24): the user confirmed the takeover is fun and chose the drift event for the finale. Tire smoke, skid marks, and the burnout check carry over into M2. Playtest record (reconstructed): `docs/playtests/2026-10-24-m1-gate.md`.
3. M2, week 5. Mission subsystem, loop state, progress subsystem and save game, heat, raid timer, HUD (heat meter, score, objective text, minimap).
   - Acceptance:
     - A full loop runs in `L_Greybox` through `UMissionSubsystem`: Idle, BreakIn, Getaway, Takeover, Chop, Dump, Complete. Every phase but Takeover is a stub that completes on a debug command until M3 to M5. Phase transitions are covered by automation tests.
     - Heat: 0 to 100 with 4 tiers and hysteresis. Crowd hits and takeover duration add heat, and heat decays while unseen (line-of-sight stub until M4). Tier changes, hysteresis, and decay are covered by tests.
     - Raid timer: tier 3 during the takeover starts a countdown. Leaving the arena before it ends banks rep converted from score. Running out means arrest: the loop fails with partial rewards (banked rep kept, unbanked money x `FailureRewardFraction`, default 0.25). Reward math is covered by tests.
     - Money, rep, and loops completed survive a PIE restart (saved on loop end and on quit).
     - HUD: heat meter, score and combo, objective text, raid countdown, and a minimap (top-down scene capture around the car with a player arrow and an objective marker; M4 adds police blips). The debug HUD moves behind a console variable.
     - Heat rates and raid timing go to the user for a playtest. They are not marked done from code.
4. M3, weeks 6 to 7. Break-in phase, on-foot character, pawn swap into the car.
5. M4, weeks 8 to 9. Getaway pursuit and dump phase.
6. M5, week 10. Chop menu and the takeover event.
7. M6, weeks 11 to 13. Art pass with City Sample assets, audio, settings menu, gamepad menu navigation, crash reporting, packaging (confirm no MCP plugin code ships), store assets.

## 9. Out of scope for Early Access

Multiplayer, multiple cars, car customization, a second map, progression trees, modular chop meshes, large crowd scale, story or cutscenes, console builds.

## 10. Working rules

See `AGENTS.md` and `CLAUDE.md`. Update this plan and `CHANGELOG.md` when scope changes.

## 11. Open questions for the user

- Working title and fictional name for the hero car. The code name stays `Takeover` either way.
- Art direction: realistic (City Sample look) or stylized. Affects the art pass and disk use.
- Ambient traffic during getaway and dump: none for EA, or simple spline traffic. Traffic adds time to M4.
- Crash reporting provider. Any third-party SDK is ask-first. Decide before M6.
- Early Access price point.
- Is Unreal already installed, and on what hardware (CPU, GPU, RAM, free disk)? Determines whether City Sample Crowd and Buildings are viable.

Resolved:

- Engine version: 5.8 (2026-10-23).
- Takeover finale: drift event (2026-10-24, M1 gate).
- Partial reward on failure: banked rep is kept, unbanked money x `FailureRewardFraction` (default 0.25) (2026-10-24).
- Minimap: in M2 as a simple top-down capture (2026-10-24).

## 12. Revision history

- 2026-10-27: Agent docs and repo administration. `main` is the default branch with M0 and M1 on it. Working rules split into `AGENTS.md` (any agent) and `CLAUDE.md` (Claude Code, imports `AGENTS.md`). Added the plugin and skill stack (Superpowers, 26 quodsoler Unreal skills) with Unreal-specific overrides, the learning cycle (`docs/learning/`), ADRs 0001 to 0014 linked from the sections they shape, playtest logs, issue and PR templates, CI repo checks, and git and Claude Code hooks that block third-party Content commits. Decided: the full Superpowers plugin on every model, one or two your-turn tasks per milestone, and learning notes only for features that introduce a new concept. Folder layout and testing sections updated.
- 2026-10-24: Third-party content (City Sample, Fab, vehicle template) leaves git, because the repo is public. `Tools/VendorContent` restores it from the staging project and the engine install using a committed manifest. Removed from the unpushed M1 history before the first push.
- 2026-10-24: M1 gate passed, drift event chosen. M2 acceptance criteria added. Failure reward rule and minimap approach resolved.
- 2026-10-24: M1 editor assets created through MCP (status under M1). Stand-in hero car is the vehicle template's SportsCar.
- 2026-10-24: Enabled `LiveCodingToolset` (Editor target only), since `AllToolsets` does not include it and `CLAUDE.md` relies on `LiveCodingToolset.CompileLiveCoding`. M1 code status updated: first compile clean, 24 `Takeover.*` tests pass.
- 2026-10-23 (M1 start): Scoring spec filled in to match the first-pass code: max drift angle, speed factor, proximity applies to all scoring states, combo cap and multiplier, donut throttle requirement, collision filtering. M1 editor-side asset list added. Hero car gets its own wheel classes.
- 2026-10-23: Engine 5.7 to 5.8, since Unreal MCP ships with 5.8. Added the development environment section (local vs cloud sessions, toolchain, LFS budget, MCP cautions). Added the world and lifetime model, `FMissionLoopState`, `UMissionConfig`, and the player and input architecture. Defined slip angle and scoring state priority, and extended the combo to all scoring states. Moved gamepad bindings from M6 to M0. M4 extended from one week to two, total schedule 13 weeks. Anchored the Steam timeline to the release week and added the AI disclosure rule. Folder layout: `Takeover/` renamed to `Arena/` to avoid `Source/Takeover/Takeover/`, added `Player/`, `Tests/`, `Content/Takeover/Input/`, `Content/Takeover/UI/`, and third-party content stays at its migrated path. Corrected the tone reference to Hood Warfare 2. Added version caveats for Easy Vehicle System and Drift AI. Working rules moved to `CLAUDE.md`.

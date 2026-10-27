# Changelog

All notable changes to this project. Format follows [Keep a Changelog](https://keepachangelog.com/en/1.1.0/).

## [Unreleased]

### Added

- Agent tooling: Superpowers plugin enabled; 26 of the 31 `quodsoler/unreal-engine-skills` vendored under `.claude/skills/` at `f3742d7` (MIT); `.agents/ue-project-context.md` for those skills. Versions, review findings, and update steps in `docs/tooling.md`.
- `AGENTS.md` with the rules for any coding agent. `CLAUDE.md` imports it and adds the precedence order, Superpowers overrides (no worktrees, TDD scope, subagent limits), and the learning cycle commands.
- Learning cycle in `docs/learning/`: process, concept map, glossary, note template, notes 0001 (repo skeleton) and 0002 (scoring model); `tutor` subagent; `/walkthrough`, `/learning-note`, `/your-turn`, `/quiz`, `/retro`. AI disclosure: written by Claude; developer documentation, does not ship in the game.
- Playtest log convention in `docs/playtests/`, with the M1 gate entry reconstructed from commits for Rashun to correct.
- ADRs 0001 to 0014 in `docs/decisions/`, linked from `PLAN.md`.
- Hooks: `Tools/githooks/pre-commit` blocks commits that stage `Content/` outside the first-party folders, with Git LFS's own hooks alongside it; Claude Code hooks block the same commits and force-adds, warn before Unreal MCP calls with uncommitted `Content/` or `Config/` changes, and limit the tutor's writes. `/mcp-checkpoint` command.
- CI (`.github/workflows/repo-checks.yml`): license boundary, manifest completeness, LFS pointers, file size, and Markdown links, with tests.
- Issue templates (feature, bug, playtest feedback), a PR template, and `.github/scripts/bootstrap_github.py` for labels, milestones, M2 issues, the default branch, and branch protection.
- `docs/windows-checklist.md` for work only the Windows machine can verify.
- City Sample hero buildings around the plaza in `L_Greybox`: the packed (`BPP_`) versions of `Bldg_Hero_Mid_NYG_Modern_A01`, `Bldg_Hero_NYG_Modern_Square_A01`, `Bldg_Hero_Low_CHG_Modern_A01`, and `Bldg_Hero_Tower_CHE_Modern_A01` at 10 spots, 7 of them with their ground-floor packs. 40 warm uplights wash the plaza-facing facades, because the City Sample materials have no lit windows at night. About 3.1 GiB. The Level Instance versions are migrated too, but they render only as LevelStreaming because City Sample levels are not One File Per Actor. City Sample content is Epic's, licensed for Unreal Engine projects only.
- City art pass: 420 packages (1.96 GiB) migrated by dependency closure from a City Sample staging project. Includes the City Sample modular building kit, street props, curbs, and road surfaces; a Fab street props pack; a high-poly tree; and Pack_Bonus surface sets. 124 props and trees dress the plaza sidewalks and avenue. A nebula star dome and the City Sample skyline cards were tried and dropped, and are not in the repo.
- `SM_RoadMarkings` now marks the plaza as an intersection: lane lines, zebra crosswalks, stop bars, and edge lines inside the crowd ring.
- `takeover.Telemetry` console variable (on by default): 4 Hz log of fps, speed, gear, RPM, inputs, slip, scoring state, and per-wheel contact and slip.
- Greybox night city around the takeover plaza in `L_Greybox`: avenues, 16 sidewalk blocks, 68 buildings, merged lane markings, 40 street lamps (`SM_StreetLamp`), 6 plaza floodlights, 47 neon pieces, moonlight, fog, and a fixed-exposure post-process volume. Materials `M_Env_Surface` and `M_Env_Facade` (procedural lit windows) with 13 instances under `Content/Takeover/Environment/`. Built from engine basic shapes. **AI disclosure:** the layout and the procedural materials were authored by Claude through Unreal MCP. They are placeholders; if any of it ships, declare it in the Steam content survey.
- M1 editor assets, created through Unreal MCP: `IA_Throttle`, `IA_Brake`, `IA_Steer`, `IA_LookAround`, `IA_Handbrake`, `IA_ResetVehicle`, and `IMC_Vehicle` (keyboard/mouse and gamepad; handbrake on Space, Gamepad A, and Gamepad RB); `DA_InputConfig`, `DA_Handling_Drift`, `DA_Scoring`; `BP_HeroCar` with its own `BP_HeroCar_WheelFront`/`BP_HeroCar_WheelRear`, `BP_Arena`, `BP_TakeoverPlayerController`, `BP_TakeoverGameMode`; `L_Greybox` (World Partition, One File Per Actor, flat 500 m ground slab) set as the editor startup and game default map.
- Stand-in hero car: the UE 5.8 vehicle template's SportsCar at `/Game/Vehicles/SportsCar`, copied unchanged from the engine install. Not in git; `Tools/VendorContent` restores it from the engine. Engine template content, not AI-generated.
- `LiveCodingToolset` plugin enabled for the Editor target, so Live Coding compiles can run over Unreal MCP.
- Editor-generated `Config/DefaultEngine.ini` and `Config/DefaultInput.ini` (Enhanced Input default classes).
- M1 code core, first pass, compiles clean on UE 5.8.4 with all `Takeover.*` tests passing: `ATakeoverGameMode`, `ATakeoverPlayerController` (mapping context per pawn), `UTakeoverInputConfig`, `ATakeoverVehiclePawn` (Enhanced Input driving, chase camera, reset), `UTakeoverHandlingProfile` (runtime Chaos setters), `FTakeoverScoringModel` and `UTakeoverScoringConfig` (drift, donut, burnout, proximity, combo), `UTakeoverScoringComponent` (vehicle sampling, collision and crowd hit detection), `ATakeoverArena` (crowd spline with placeholder meshes), `ATakeoverDebugHUD`, `Takeover.Scoring.Model` automation spec.
- M0 repo setup: `Takeover.uproject` targeting UE 5.8, `Takeover` C++ module skeleton, game and editor targets, folder layout, git and Git LFS config, Unreal MCP config, `CLAUDE.md`, smoke automation test.

### Changed

- `main` is the new branch line, created from the M1 work (`dab4dd3`). Work happens on feature branches merged by pull request after a walkthrough.
- Working rules moved from `CLAUDE.md` to `AGENTS.md`, unchanged except for additions.
- `.gitignore` ignores Python caches; `.gitattributes` keeps hook scripts LF.
- Third-party content (City Sample, Fab packs, the vehicle template; 1068 files, 5.09 GiB) is no longer committed, because the repo is public and those licenses don't allow publishing the source assets. `Tools/VendorContent/manifest.json` lists each file with its source and SHA-256. `vendor_content.py restore` copies them from the staging project and the engine install, `verify` checks them, and `record` updates the manifest after a migration. `.gitignore` tracks only `Content/Takeover/` and the level's per-actor folders. Removed from the unpushed M1 history before the first push.
- Hero car automatic gearbox shifts up at 5600 RPM (was 6000), because takeover arenas have short straights.
- `PLAN.md` refined. Engine target moved from 5.7 to 5.8. See the plan's revision history.

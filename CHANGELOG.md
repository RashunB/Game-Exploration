# Changelog

All notable changes to this project. Format follows [Keep a Changelog](https://keepachangelog.com/en/1.1.0/).

## [Unreleased]

### Added

- City Sample hero buildings around the plaza in `L_Greybox`: the packed (`BPP_`) versions of `Bldg_Hero_Mid_NYG_Modern_A01`, `Bldg_Hero_NYG_Modern_Square_A01`, `Bldg_Hero_Low_CHG_Modern_A01`, and `Bldg_Hero_Tower_CHE_Modern_A01` at 10 spots, 7 of them with their ground-floor packs. 40 warm uplights wash the plaza-facing facades, because the City Sample materials have no lit windows at night. About 3.1 GiB in LFS. The Level Instance versions are migrated too, but they render only as LevelStreaming because City Sample levels are not One File Per Actor. City Sample content is Epic's, licensed for Unreal Engine projects only.
- City art pass: 440 packages (2.53 GiB) migrated by dependency closure from a City Sample staging project. Includes the City Sample modular building kit, street props, curbs, and road surfaces; a Fab street props pack; a high-poly tree; the Animated Nebulas star dome; and Pack_Bonus surface sets. 124 props and trees dress the plaza sidewalks and avenue. The nebula dome and the City Sample skyline cards are unused for now.
- `SM_RoadMarkings` now marks the plaza as an intersection: lane lines, zebra crosswalks, stop bars, and edge lines inside the crowd ring.
- `takeover.Telemetry` console variable (on by default): 4 Hz log of fps, speed, gear, RPM, inputs, slip, scoring state, and per-wheel contact and slip.
- Greybox night city around the takeover plaza in `L_Greybox`: avenues, 16 sidewalk blocks, 68 buildings, merged lane markings, 40 street lamps (`SM_StreetLamp`), 6 plaza floodlights, 47 neon pieces, moonlight, fog, and a fixed-exposure post-process volume. Materials `M_Env_Surface` and `M_Env_Facade` (procedural lit windows) with 13 instances under `Content/Takeover/Environment/`. Built from engine basic shapes. **AI disclosure:** the layout and the procedural materials were authored by Claude through Unreal MCP. They are placeholders; if any of it ships, declare it in the Steam content survey.
- M1 editor assets, created through Unreal MCP: `IA_Throttle`, `IA_Brake`, `IA_Steer`, `IA_LookAround`, `IA_Handbrake`, `IA_ResetVehicle`, and `IMC_Vehicle` (keyboard/mouse and gamepad; handbrake on Space, Gamepad A, and Gamepad RB); `DA_InputConfig`, `DA_Handling_Drift`, `DA_Scoring`; `BP_HeroCar` with its own `BP_HeroCar_WheelFront`/`BP_HeroCar_WheelRear`, `BP_Arena`, `BP_TakeoverPlayerController`, `BP_TakeoverGameMode`; `L_Greybox` (World Partition, One File Per Actor, flat 500 m ground slab) set as the editor startup and game default map.
- Stand-in hero car: the UE 5.8 vehicle template's SportsCar at `/Game/Vehicles/SportsCar`, copied unchanged from the engine install. Engine template content, not AI-generated.
- `LiveCodingToolset` plugin enabled for the Editor target, so Live Coding compiles can run over Unreal MCP.
- Editor-generated `Config/DefaultEngine.ini` and `Config/DefaultInput.ini` (Enhanced Input default classes).
- M1 code core, first pass, compiles clean on UE 5.8.4 with all `Takeover.*` tests passing: `ATakeoverGameMode`, `ATakeoverPlayerController` (mapping context per pawn), `UTakeoverInputConfig`, `ATakeoverVehiclePawn` (Enhanced Input driving, chase camera, reset), `UTakeoverHandlingProfile` (runtime Chaos setters), `FTakeoverScoringModel` and `UTakeoverScoringConfig` (drift, donut, burnout, proximity, combo), `UTakeoverScoringComponent` (vehicle sampling, collision and crowd hit detection), `ATakeoverArena` (crowd spline with placeholder meshes), `ATakeoverDebugHUD`, `Takeover.Scoring.Model` automation spec.
- M0 repo setup: `Takeover.uproject` targeting UE 5.8, `Takeover` C++ module skeleton, game and editor targets, folder layout, git and Git LFS config, Unreal MCP config, `CLAUDE.md`, smoke automation test.

### Changed

- Hero car automatic gearbox shifts up at 5600 RPM (was 6000), because takeover arenas have short straights.
- `PLAN.md` refined. Engine target moved from 5.7 to 5.8. See the plan's revision history.

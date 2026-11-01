# UE Project Context

*Engine: UE 5.8 · Last updated: 2026-10-27*

Read by the vendored quodsoler `ue-*` skills before they give advice. Facts here come from the project files. Design and scope live in `PLAN.md`, working rules in `AGENTS.md`; when this file and those disagree, they win and this file gets fixed.

## Engine & Project
**Engine version:** 5.8 (`EngineAssociation` "5.8"), launcher build, 5.8.4 at last build
**Project name:** Takeover (code name; store title not chosen) · **Type:** game · **Genre:** single-player third-person crime/driving
**Description:** One mission loop (break-in, getaway, street takeover, chop, dump) built around Chaos vehicle drifting.
**Target platforms:** Windows only (`TargetPlatforms` in `Takeover.uproject`)

## Modules
**Primary game module:** Takeover

| Module | Host type | Public deps | Private deps | Notes |
|---|---|---|---|---|
| Takeover | Runtime | Core, CoreUObject, Engine, InputCore, EnhancedInput, ChaosVehicles, PhysicsCore | none | All gameplay code. `PublicIncludePaths` adds the module root, so headers are included by path: `#include "Vehicle/TakeoverVehiclePawn.h"`. `PCHUsage = UseExplicitOrSharedPCHs`. |

No editor module and no project plugins under `Plugins/`.

## Plugins
| Plugin | Maturity | Used for |
|---|---|---|
| ChaosVehiclesPlugin | stable | hero car, later police and traffic |
| EnhancedInput | stable | all input |
| StateTree | stable | homeowner AI (M3), not used yet |
| GameplayStateTree | stable | `UStateTreeAIComponent` for AI (M3), not used yet |
| ModelingToolsEditorMode | stable | Editor target only |
| ModelContextProtocol | Experimental | Editor target only; Unreal MCP server for Claude Code |
| AllToolsets | Experimental | Editor target only; MCP tool set |
| LiveCodingToolset | Experimental | Editor target only; Live Coding over MCP |

**In-house plugins:** none
**Fab / marketplace:** City Sample (buildings, vehicles planned), Fab street props. Not in git: the repo is public and those licenses forbid publishing source assets. `Tools/VendorContent/manifest.json` lists them and `vendor_content.py restore` rebuilds them. Never edit migrated third-party assets.

## Coding conventions
**Prefixes:** Epic standard `U`/`A`/`F`/`E`/`I`/`T`, `b` on bools, tabs, PascalCase
**Object references:** `TObjectPtr` in `UPROPERTY`
**API macro style:** `TAKEOVER_API` per declaration
**Log categories:** `LogTakeover` (declared in `Source/Takeover/Takeover.h`)
**Assertions:** [unknown] (current code uses `CastChecked` once and warns via `UE_LOG` otherwise)
**Header layout:** flat per feature folder (`Arena/`, `Core/`, `Player/`, `Vehicle/`, `UI/`, `Tests/`), no Public/Private split
**Enforced rules:** every tuning value is `UPROPERTY(EditDefaultsOnly)` or lives in a DataAsset. Testable math lives in plain structs with no `UWorld` (example: `FTakeoverScoringModel`). Blueprints are thin data subclasses of C++ classes.

## Gameplay framework
| Role | Class |
|---|---|
| GameMode | `ATakeoverGameMode` (`BP_TakeoverGameMode`) |
| GameState | engine default; none planned |
| PlayerController | `ATakeoverPlayerController` (`BP_TakeoverPlayerController`); owns heat from M2 |
| PlayerState | engine default; none planned |
| Pawn / Character | `ATakeoverVehiclePawn` (`AWheeledVehiclePawn`, `BP_HeroCar`); `ATakeoverCharacter` planned for M3 |
| GameInstance | engine default |
| HUD | `ATakeoverDebugHUD` (Canvas debug readout) |

**Subsystems (planned, M2):** `UMissionSubsystem` (`UTickableWorldSubsystem`) owns the loop and phase transitions; `UTakeoverProgressSubsystem` (`UGameInstanceSubsystem`) owns money, rep, and saves.

Single-player: do not put state on GameState or PlayerState for replication reasons. Score lives on the vehicle's `UTakeoverScoringComponent`, heat on the controller, progress in the game instance subsystem (see `PLAN.md` section 4).

## GAS
**Enabled:** no. Not planned for Early Access. Do not suggest GameplayAbilities, AttributeSets, or GameplayEffects.

## Networking
**Model:** single-player only. No replication, RPCs, `HasAuthority` branches, or `GetLifetimeReplicatedProps`. Ignore networking sections in any skill.

## Input
**Stack:** EnhancedInput
**Mapping contexts:** `/Game/Takeover/Input/IMC_Vehicle` (`IMC_OnFoot` and `IMC_Menu` planned) · **Added by:** `ATakeoverPlayerController::ApplyMappingContextFor`, per possessed pawn
**Asset references:** `UTakeoverInputConfig` DataAsset (`DA_InputConfig`)

## UI
**Stack:** debug Canvas HUD now; UMG planned for M2 (heat meter, score, objective, minimap). CommonUI not planned.

## AI
**Stack:** StateTree / GameplayStateTree with AI Perception for the homeowner (M3). Police pursuit on Chaos along a road spline network (M4). No Behavior Trees, no Mass.

## Streaming
**Model:** one persistent World Partition map with One File Per Actor (`/Game/Takeover/Maps/L_Greybox`). No level travel during a loop. Per-actor files live under `Content/__ExternalActors__/` and `Content/__ExternalObjects__/`.

## Saves
**SaveGame classes:** `UTakeoverSaveGame` planned for M2 (money, rep, loops completed, settings) · **Slots:** [unknown] · **Versioning:** [unknown]

## Build
**Targets:** Game (`Takeover.Target.cs`), Editor (`TakeoverEditor.Target.cs`), both `BuildSettingsVersion.Latest` and `EngineIncludeOrderVersion.Latest`
**Defines:** none
**Third-party:** none
**Platform notes:** Windows only; builds and tests run only on the Windows dev machine
**Engine modifications:** none

## Team
**Size & roles:** one developer (Rashun), with Claude Code writing most code
**Source control:** Git with Git LFS, no LFS locking · **Branching:** one branch per feature off `main`, merged by PR after a walkthrough
**Review bar:** see `AGENTS.md` · **Docs:** `PLAN.md`, `AGENTS.md`, `CLAUDE.md`, `docs/`

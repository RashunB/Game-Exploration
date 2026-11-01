# Concept map

Every Unreal and game dev concept this repo uses, where to see it, and where Rashun stands with it.

Status: `not yet` (not in code he has looked at), `seen` (in the repo, not yet taught), `explained` (he has had the walkthrough or read the note), `can explain` (he says he could explain it to someone else; only Rashun sets this). The notes column records quiz weak spots and open questions.

## Project and build

| Concept | Where in this repo | Status | Note | Notes |
| --- | --- | --- | --- | --- |
| `.uproject` file and project plugins | `Takeover.uproject` | seen | [0001](notes/0001-repo-skeleton.md) | |
| Modules and `Build.cs` dependencies | `Source/Takeover/Takeover.Build.cs`, `Source/Takeover/Takeover.cpp` | seen | [0001](notes/0001-repo-skeleton.md) | |
| Build targets (Game vs Editor) | `Source/Takeover.Target.cs`, `Source/TakeoverEditor.Target.cs` | seen | [0001](notes/0001-repo-skeleton.md) | |
| Unreal Build Tool and `Build.bat` | `AGENTS.md` (Commands) | seen | [0001](notes/0001-repo-skeleton.md) | |
| Live Coding vs full builds | `CLAUDE.md` (Unreal MCP), `Takeover.uproject` (`LiveCodingToolset`) | seen | [0001](notes/0001-repo-skeleton.md) | |
| Log categories (`UE_LOG`) | `Source/Takeover/Takeover.h`, `Source/Takeover/Takeover.cpp` | seen | [0001](notes/0001-repo-skeleton.md) | |
| Automation tests and specs | `Source/Takeover/Tests/TakeoverSmokeTest.cpp`, `Source/Takeover/Tests/TakeoverScoringModelSpec.cpp` | seen | [0001](notes/0001-repo-skeleton.md), [0002](notes/0002-scoring-model.md) | |
| Headless test runs (`-nullrhi`) | `AGENTS.md` (Commands) | seen | [0001](notes/0001-repo-skeleton.md) | |
| Git LFS for binary assets | `.gitattributes` | seen | [0001](notes/0001-repo-skeleton.md) | |
| Unreal MCP editor control | `.mcp.json`, `Config/DefaultEditorPerProjectUserSettings.ini`, `PLAN.md` section 3 | seen | [0001](notes/0001-repo-skeleton.md) | |
| Config `.ini` files | `Config/DefaultEngine.ini`, `Config/DefaultInput.ini`, `Config/DefaultGame.ini` | seen | | |
| Vendor content manifest | `Tools/VendorContent/vendor_content.py`, `Tools/VendorContent/manifest.json` | seen | 0007 | |
| Dependency closure of packages | `Tools/VendorContent/dep_closure.py`, `Tools/VendorContent/migrate.py` | seen | 0007 | |

## Object model and reflection

| Concept | Where in this repo | Status | Note | Notes |
| --- | --- | --- | --- | --- |
| UObject, UCLASS, and garbage collection | `Source/Takeover/Arena/TakeoverScoringConfig.h` | seen | | |
| Reflection and generated headers (`.generated.h`, `GENERATED_BODY`) | `Source/Takeover/Arena/TakeoverScoringTypes.h` | seen | [0002](notes/0002-scoring-model.md) | |
| USTRUCT vs plain C++ struct | `Source/Takeover/Arena/TakeoverScoringTypes.h` (`FTakeoverScoringParams`), `Source/Takeover/Arena/TakeoverScoringModel.h` (`FTakeoverScoringModel`) | seen | [0002](notes/0002-scoring-model.md) | |
| UPROPERTY specifiers and `meta` clamps | `Source/Takeover/Arena/TakeoverScoringTypes.h` | seen | [0002](notes/0002-scoring-model.md) | |
| UENUM | `Source/Takeover/Arena/TakeoverScoringTypes.h` (`ETakeoverScoringState`) | seen | [0002](notes/0002-scoring-model.md) | |
| UFUNCTION and Blueprint exposure | `Source/Takeover/Arena/TakeoverScoringComponent.h` | seen | 0003 | |
| `TObjectPtr` | `Source/Takeover/Arena/TakeoverScoringComponent.h` | seen | 0003 | |
| Class default objects (CDO) and `GetDefault` | `Source/Takeover/Arena/TakeoverScoringComponent.cpp` (`GetActiveConfig`) | seen | 0003 | |
| `TAKEOVER_API` export macro | `Source/Takeover/Arena/TakeoverScoringModel.h` | seen | [0002](notes/0002-scoring-model.md) | |
| DataAssets for tuning | `Source/Takeover/Arena/TakeoverScoringConfig.h`, `Source/Takeover/Vehicle/TakeoverHandlingProfile.h`, `Content/Takeover/Data/` | seen | 0003 | |
| Blueprint subclasses of C++ classes | `Content/Takeover/Blueprints/BP_HeroCar.uasset`, `Source/Takeover/Vehicle/TakeoverVehiclePawn.h` (`UCLASS(Abstract)`) | seen | 0004 | |
| Delegates (dynamic multicast) | `Source/Takeover/Arena/TakeoverScoringComponent.h` (`OnCrowdHit`) | seen | 0003 | |
| Console variables | `Source/Takeover/Vehicle/TakeoverVehiclePawn.cpp` (`takeover.Telemetry`) | seen | 0004 | |

## Gameplay framework

| Concept | Where in this repo | Status | Note | Notes |
| --- | --- | --- | --- | --- |
| Actors, components, and ticking | `Source/Takeover/Arena/TakeoverScoringComponent.cpp`, `Source/Takeover/Vehicle/TakeoverVehiclePawn.cpp` | seen | 0003 | |
| GameMode | `Source/Takeover/Core/TakeoverGameMode.h` | seen | 0005 | |
| PlayerController, Pawn, and possession | `Source/Takeover/Player/TakeoverPlayerController.cpp` | seen | 0005 | |
| HUD (Canvas) | `Source/Takeover/UI/TakeoverDebugHUD.cpp` | seen | | |
| Spring arm and camera components | `Source/Takeover/Vehicle/TakeoverVehiclePawn.cpp` (constructor) | seen | 0004 | |
| Enhanced Input (Input Actions, Mapping Contexts) | `Source/Takeover/Player/TakeoverInputConfig.h`, `Content/Takeover/Input/` | seen | 0005 | |
| Splines | `Source/Takeover/Arena/TakeoverArena.cpp` | seen | 0006 | |
| Instanced static meshes | `Source/Takeover/Arena/TakeoverArena.h` (`CrowdPlaceholders`) | seen | 0006 | |
| Hit events and collision filtering | `Source/Takeover/Arena/TakeoverScoringComponent.cpp` (`HandleOwnerHit`) | seen | 0003 | |

## Vehicles and game math

| Concept | Where in this repo | Status | Note | Notes |
| --- | --- | --- | --- | --- |
| Vectors, dot product, ground-plane projection | `Source/Takeover/Arena/TakeoverScoringModel.cpp` (`SlipAngleDeg`), `Source/Takeover/Arena/TakeoverScoringComponent.cpp` (`BuildSample`) | seen | [0002](notes/0002-scoring-model.md) | |
| Slip angle | `Source/Takeover/Arena/TakeoverScoringModel.cpp` | seen | [0002](notes/0002-scoring-model.md) | |
| Yaw rate | `Source/Takeover/Arena/TakeoverScoringModel.cpp` (`Classify`), `Source/Takeover/Arena/TakeoverScoringComponent.cpp` | seen | [0002](notes/0002-scoring-model.md) | |
| Frame-rate-independent accumulation (delta time) | `Source/Takeover/Arena/TakeoverScoringModel.cpp` (`Tick`) | seen | [0002](notes/0002-scoring-model.md) | |
| Unreal units (cm, cm/s) | `Source/Takeover/Arena/TakeoverScoringModel.cpp` (`KmhPerCmPerSec`) | seen | [0002](notes/0002-scoring-model.md) | |
| Chaos wheeled vehicles | `Source/Takeover/Vehicle/TakeoverVehiclePawn.cpp` | seen | 0004 | |
| Wheel classes and per-wheel setup (UE-274631) | `Content/Takeover/Blueprints/BP_HeroCar_WheelFront.uasset`, `BP_HeroCar_WheelRear.uasset` | seen | 0004 | |
| Runtime handling setters vs creation-time values | `Source/Takeover/Vehicle/TakeoverVehiclePawn.cpp` (`ApplyHandlingProfile`), `Source/Takeover/Vehicle/TakeoverHandlingProfile.h` | seen | 0004 | |
| Tire friction, slip threshold, gearbox shift logic | commits `8853f62`, `a389d39`, `7f8d344` | seen | 0004 | |
| Angular damping | `Source/Takeover/Vehicle/TakeoverVehiclePawn.cpp` (`Tick`), commit `b4b664e` | seen | 0004 | |

## World and levels

| Concept | Where in this repo | Status | Note | Notes |
| --- | --- | --- | --- | --- |
| World Partition | `Content/Takeover/Maps/L_Greybox.umap` | seen | 0006 | |
| One File Per Actor (`__ExternalActors__`, `__ExternalObjects__`) | `Content/__ExternalActors__/`, `Content/__ExternalObjects__/` | seen | 0006 | |
| Packed level actors (`BPP_`) | `CHANGELOG.md` (City Sample hero buildings) | seen | 0006 | |

## Coming in M2

| Concept | Where it will appear | Status | Note | Notes |
| --- | --- | --- | --- | --- |
| World subsystems (`UTickableWorldSubsystem`) | `UMissionSubsystem`, `PLAN.md` section 4 | not yet | | |
| Game instance subsystems | `UTakeoverProgressSubsystem` | not yet | | |
| State machines (phase objects) | `UMissionPhase`, `PLAN.md` section 4 | not yet | | |
| Hysteresis | `UHeatComponent` tiers | not yet | | |
| Delegates for phase events | `OnPhaseComplete`, `OnPhaseFailed` | not yet | | |
| SaveGame | `UTakeoverSaveGame` | not yet | | |
| UMG widgets with C++ base classes | `Source/Takeover/UI/` | not yet | | |
| Scene capture for the minimap | M2 HUD | not yet | | |
| Niagara particle systems | tire smoke, skid marks | not yet | | |

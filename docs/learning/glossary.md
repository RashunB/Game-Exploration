# Glossary

Plain definitions, each tied to where the term shows up in this repo. Alphabetical. `/learning-note` adds terms as notes introduce them.

- Automation test: Unreal's built-in test framework. Tests are C++ classes registered by macro with a dotted name (`Takeover.Scoring.Model`), discovered at startup, and run from the editor or the command line. See `Source/Takeover/Tests/`.
- Automation spec: the Describe/It style of automation test (`BEGIN_DEFINE_SPEC`), closer to RSpec or Jasmine than to xUnit. `Source/Takeover/Tests/TakeoverScoringModelSpec.cpp`.
- Build.cs: a C# file that tells Unreal Build Tool how to compile one module: its dependencies, include paths, and precompiled header use. Unreal's equivalent of a per-package build manifest. `Source/Takeover/Takeover.Build.cs`.
- Combo: the takeover score multiplier. It rises one step per 2 seconds of continuous scoring, caps at x10, and breaks on a collision or after 1.5 seconds without scoring. `FTakeoverScoringModel::Tick`.
- DataAsset: an asset whose class is a C++ `UDataAsset` subclass, edited in the editor like a form. Used here for every tuning value so Rashun can tune without recompiling. `Source/Takeover/Arena/TakeoverScoringConfig.h`, `Content/Takeover/Data/`.
- Delta time: seconds since the last frame. Rates are multiplied by it so the result doesn't depend on frame rate. `DeltaSeconds` in `FTakeoverScoringModel::Tick`.
- Editor target: the build of the project that loads into the Unreal editor, with editor-only code and plugins. `Source/TakeoverEditor.Target.cs`.
- Game target: the standalone game build that ships. `Source/Takeover.Target.cs`.
- `GENERATED_BODY()`: the macro Unreal Header Tool expands into reflection boilerplate for a `UCLASS` or `USTRUCT`. Requires the matching `.generated.h` include, which must be the last include in the header.
- Git LFS: Git Large File Storage. Git stores a small pointer file and the real bytes live on the LFS server. All `.uasset` and `.umap` files go through it. `.gitattributes`.
- Live Coding: Unreal's hot patching of C++ function bodies into a running editor. It can't change class layouts, which need a full build with the editor closed.
- Module: a unit of compiled C++ in Unreal, roughly a shared library with a Build.cs. This project has one, `Takeover`. `Source/Takeover/`.
- Proximity multiplier: score multiplier for performing close to the crowd: 1 at 15 m from the crowd line, rising to 3 at 2 m. `FTakeoverScoringModel::ProximityMultiplier`.
- Reflection: Unreal's runtime type information, generated from `UCLASS`, `USTRUCT`, `UENUM`, `UPROPERTY`, and `UFUNCTION` macros. It powers the editor's property panels, Blueprints, serialization, and garbage collection.
- Slip angle: the angle between where the car points and where it is actually moving, in the ground plane. Near 0 when driving straight, large in a drift, 180 when reversing. `FTakeoverScoringModel::SlipAngleDeg`.
- Smoke test: a minimal test that proves the pipeline works end to end, here that the module loads. `Source/Takeover/Tests/TakeoverSmokeTest.cpp`.
- Unreal Build Tool (UBT): the C# program that reads the target and Build.cs files and drives the compiler. `Build.bat` in `AGENTS.md` runs it.
- Unreal MCP: Epic's Model Context Protocol server inside the editor, which lets Claude Code place actors, create assets, and compile over HTTP on 127.0.0.1:8000. `.mcp.json`.
- Unreal units: 1 unit is 1 cm. Velocities are cm/s; 1 cm/s is 0.036 km/h. `KmhPerCmPerSec` in `Source/Takeover/Arena/TakeoverScoringModel.cpp`.
- `UPROPERTY`: marks a member for reflection. Specifiers say who can edit it (`EditDefaultsOnly`: only on class defaults and assets, not on placed instances) and `meta` adds editor hints like `ClampMin`. `Source/Takeover/Arena/TakeoverScoringTypes.h`.
- `USTRUCT`: a struct registered with reflection so it can be a `UPROPERTY`, edited in the editor, and saved. Not garbage collected and has no `UFUNCTION`s. `FTakeoverScoringParams`.
- Yaw rate: how fast the car rotates about its vertical axis, in degrees per second. High yaw rate at low speed is a donut. `FTakeoverVehicleSample::YawRateDegPerSec`.

# 0002: Scoring model

Branch or commits: `e15a785` (M1 code core) · Written: 2026-10-27 · Concept map rows: USTRUCT vs plain C++ struct, UPROPERTY specifiers and `meta` clamps, UENUM, Reflection and generated headers, `TAKEOVER_API`, Vectors and ground-plane projection, Slip angle, Yaw rate, Frame-rate-independent accumulation, Unreal units, Automation tests and specs

## What this feature does

It decides, every frame, whether the car is drifting, doing a donut, doing a burnout, or none of those, and turns that into points with a proximity bonus and a combo multiplier. In code terms: a plain C++ struct takes one sample of vehicle state and a block of tuning values, and returns the state, the points, and combo events. It never touches the game world, so 23 automation tests drive it directly, headless.

## Files to read, in order

1. `Source/Takeover/Arena/TakeoverScoringTypes.h` (whole file): the scoring state enum and every tuning value. Data first.
2. `Source/Takeover/Arena/TakeoverScoringModel.h` (whole file): the input sample, the per-tick result, and the model's interface.
3. `Source/Takeover/Arena/TakeoverScoringModel.cpp` (whole file): the math. Read `SlipAngleDeg` (line 11), `Classify` (line 29), then `Tick` (line 93).
4. `Source/Takeover/Tests/TakeoverScoringModelSpec.cpp` (whole file): the tests, which double as worked examples.
5. Wiring, briefly: `Source/Takeover/Arena/TakeoverScoringConfig.h:15` (where the tuning values live as an asset) and `Source/Takeover/Arena/TakeoverScoringComponent.cpp:62` (the one line where the game calls the model). Note 0003 covers the component.

## The core idea

Keep the game rule in a plain struct that knows nothing about the world, and let a thin component feed it. This is the repo's main testing pattern (`AGENTS.md`, Testing and verification).

Most Unreal gameplay code lives in actors and components. They only exist inside a running world: they tick because the engine ticks them, and they read physics state from the engine. Testing that code means spinning up a world, spawning a car, and driving it, which is slow and flaky, and on this project impossible outside the Windows machine with a renderer. So the scoring rules are pulled out into `FTakeoverScoringModel` (`TakeoverScoringModel.h:41`), a struct with no `UCLASS`, no `UObject` base, and no pointer to a world. Its inputs are plain values: `FTakeoverVehicleSample` (`TakeoverScoringModel.h:7`) holds the car's forward direction and velocity flattened to 2D, the yaw rate, throttle, brake, distance to the crowd, and collision flags. Its output is `FTakeoverScoringTickResult` (`TakeoverScoringModel.h:25`). The infra analogy fits well: it is a pure function over a state struct, with the world as an adapter around it. The component gathers a sample from the Chaos vehicle (`TakeoverScoringComponent.cpp:115`) and hands it over (`TakeoverScoringComponent.cpp:62`). That is the only coupling.

Two kinds of struct sit side by side, and the difference is the point:

- `FTakeoverScoringParams` (`TakeoverScoringTypes.h:17`) is a `USTRUCT`. The `USTRUCT` and `GENERATED_BODY()` macros register it with Unreal's reflection system, a code generator (Unreal Header Tool) that writes the `.generated.h` file. Reflection is what lets the editor show these fields as a form on the `DA_Scoring` asset, save them, and clamp them: `UPROPERTY(EditDefaultsOnly, ..., meta = (ClampMin = "0"))` (`TakeoverScoringTypes.h:22`) means editable on class defaults and assets but not on placed instances, never below 0. Every tuning value is here so Rashun can tune without a recompile.
- `FTakeoverScoringModel` is a plain struct. It needs no editor form and no saving, so it skips reflection entirely. That keeps it cheap to construct in a test: `Model = FTakeoverScoringModel();` (`TakeoverScoringModelSpec.cpp:51`).

`TAKEOVER_API` on both (`TakeoverScoringModel.h:41`) exports the symbols from the module's DLL so other modules could call them. The `U`/`A`/`F`/`E` prefixes are Epic's naming rule: `F` for plain structs and value types, `U` for `UObject`s, `A` for actors, `E` for enums.

The math, in reading order:

- Units. Unreal measures distance in centimeters, so velocity is cm/s. 1 cm/s is 0.036 km/h (`TakeoverScoringModel.cpp:5`).
- Slip angle (`TakeoverScoringModel.cpp:11`) is the angle between where the car points and where it moves, in the ground plane. Normalize both vectors; their dot product is the cosine of the angle between them; `acos` turns that back into an angle (line 21). The clamp to [-1, 1] on line 20 guards against floating-point error pushing the cosine just past 1, where `acos` returns NaN. Below 1 cm/s the velocity direction is noise, so the angle is 0 (line 14).
- Yaw rate is how fast the car spins about its vertical axis, in degrees per second. The model doesn't compute it; the component reads it from physics.
- Classification (`TakeoverScoringModel.cpp:29`) picks exactly one state per tick, in priority order: Donut, Drift, Burnout, None. A donut is high yaw rate at low speed with throttle held (line 34). It also has a large slip angle, so if drift were checked first, every donut would score as a drift. A drift needs at least 20 km/h and a slip angle from 15 to 100 degrees (line 42); above 100 the car is spinning out or reversing. A burnout is throttle and brake together under 10 km/h (line 49).
- Points per second (`TakeoverScoringModel.cpp:71`): a drift earns (slip above 15°) x speed factor x proximity, where the speed factor is speed / 60 km/h capped at 3. Donuts and burnouts earn a flat rate x proximity. Proximity (line 59) is a linear interpolation, `Lerp`, from 1 at 15 m from the crowd line to 3 at 2 m.
- Tick (`TakeoverScoringModel.cpp:93`) is the only stateful part, and it holds just the score, the combo, and two timers (`TakeoverScoringModel.h`, private members). Everything else is a `static` function: same inputs, same answer. A collision or crowd hit breaks the combo first (line 101). Time spent scoring accumulates, and every 2 seconds the combo steps up, to a cap of 10 (line 121). Time spent in None accumulates separately, and 1.5 seconds of it breaks the combo (line 112). Points are rate x combo x `DeltaSeconds` (line 132).

`DeltaSeconds` is the time since the last frame. Multiplying rates by it is what makes the score independent of frame rate: at 30 or 120 fps, a two-second drift earns the same points, it just arrives in fewer, larger slices.

The tests (`TakeoverScoringModelSpec.cpp:39`) use Unreal's spec style: `Describe` groups and `It` cases, with `BeforeEach` (line 48) resetting the model and default params before every case. The file includes only the model header. No world, no engine objects: that is why `-nullrhi` (no renderer) is enough to run them. Helpers build samples in readable terms, `Moving(40.f, 30.f)` meaning 40 km/h with 30° of slip. The tick step is 0.25 s (line 9) on purpose: 0.25 is exactly representable in binary floating point, so four ticks sum to exactly 1.0 and the combo tests land exactly on their 2-second boundaries.

## Why it is built this way

- Plain struct plus component, not scoring logic inside the component. Alternatives: put the rules in `UTakeoverScoringComponent` and test them with a spawned world (slow, needs the engine running and a car physically drifting), or not test them (the rules are exactly the kind of thing that regresses silently while tuning). The split costs one extra struct and a sample-building function. [ADR 0006](../../decisions/0006-scoring-math-in-plain-struct.md).
- Tuning values in a `USTRUCT` on a DataAsset, not constants in code. Rashun tunes without a recompile, and a separate asset per scoring setup (a drift event could use different numbers) is possible later.
- One state per tick with a fixed priority, not overlapping states. It makes the score explainable ("you were in a donut") and stops double counting. The cost: a move that is honestly both a drift and a donut scores only as a donut.
- Combo breaks on collisions and idle time, not only on crashes. `PLAN.md` section 5.3 sets the rules. Collision filtering (soft bumps and ground contact don't count) lives in the component, because judging an impulse needs the physics hit.

## What would break if you changed X

1. Swap the order in `Classify`: move the drift check (`TakeoverScoringModel.cpp:42` to `:47`) above the donut check. Run the headless tests. Expect "prefers donut over drift" (`TakeoverScoringModelSpec.cpp:99`) to fail, because its sample has 60° of slip at 25 km/h. Revert.
2. Change the test step to `0.1f` (`TakeoverScoringModelSpec.cpp:9`) and run the tests. 0.1 has no exact binary representation, so twenty steps don't sum to exactly 2.0. Whether the sum lands just above or just below decides whether the combo tests pass, and a test whose result depends on rounding is a flaky test. Revert.
3. In the editor, open `Content/Takeover/Data/DA_Scoring`, set `ProximityMaxMultiplier` to 10, and drift along the crowd line in PIE. Expect the debug HUD score to climb much faster near the crowd, and `takeover.Telemetry` lines to show the same `slip` and `state` with a higher `score`. Revert the asset from git if you don't want to keep it.

## Check yourself

1. Why is a donut checked before a drift in `Classify`?
2. A car moves at 25 km/h with 60° of slip and a yaw rate of 200°/s, with no throttle. Which state does it score as?
3. Why is `FTakeoverScoringParams` a `USTRUCT` while `FTakeoverScoringModel` is a plain struct?
4. On the tick where the combo steps from x1 to x2, are that tick's points multiplied by 1 or by 2?
5. Why do the tests use a 0.25 s step?

<details>
<summary>Answers</summary>

1. A donut also has a large slip angle, so a drift-first check would score every donut as a drift. `TakeoverScoringModel.cpp:34`, and the test at `TakeoverScoringModelSpec.cpp:99`.
2. Drift. Without throttle it fails the donut check (`DonutMinThrottle`, `TakeoverScoringModel.cpp:36`), and at 25 km/h with 60° of slip it passes the drift check (`:42` to `:44`). The no-throttle test at `TakeoverScoringModelSpec.cpp:107` uses 10 km/h, below the drift minimum, which is why it gets None.
3. The params are edited on a DataAsset in the editor and saved, which needs reflection. The model is only ever constructed in code, so it skips reflection and stays trivial to create in a test. `TakeoverScoringTypes.h:17`, `TakeoverScoringModel.h:41`.
4. By 2. The combo update (`TakeoverScoringModel.cpp:121`) runs before the points line (`:132`).
5. 0.25 is exact in binary floating point, so accumulated ticks hit the 2-second combo boundaries exactly. `TakeoverScoringModelSpec.cpp:8` and `:9`.

</details>

## Your turn

Cover the untested branch in `ProximityMultiplier`: when an asset sets `ProximityNearDistanceCm` equal to or greater than `ProximityFarDistanceCm`, the interpolation range is zero or negative, and the function falls back to "max at or inside the near distance, 1 outside" (`TakeoverScoringModel.cpp:62`). Nothing tests that today.

- Files: `Source/Takeover/Tests/TakeoverScoringModelSpec.cpp` only.
- Done when: a new `It` case under `Describe(TEXT("ProximityMultiplier"))` passes, and fails if you temporarily delete the `if (Range <= 0.f)` block.
- Hints: `Params` is reset before every case, so set the two distances inside your `It`. Check a distance on each side of the near distance. Look at how the existing cases call `TestEqual` with a tolerance.

## Further reading

Confirmed 2026-10-27 through search results; Epic's site was not reachable from the cloud session that wrote this note.

- [Automation Spec in Unreal Engine](https://dev.epicgames.com/documentation/en-us/unreal-engine/automation-spec-in-unreal-engine)
- [Structs in Unreal Engine](https://dev.epicgames.com/documentation/unreal-engine/structs-in-unreal-engine)
- [Property Declaration (UPROPERTY)](https://dev.epicgames.com/documentation/en-us/unreal-engine/unreal-engine-uproperties)

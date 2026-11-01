# Core loop proof of concept

- Date: 2026-11-01
- Branch: `poc/core-loop` (based on `chore/agent-docs-and-admin` until PR #16 merges)
- Status: approved in chat as a POC ("treat this as poc branch"); Rashun opted out of further questions. Every choice below marked "Claude's call" is open to change.

## Intent

Rashun wants the core loop playable end to end before any systems that make it harder: steal a car from a house, drive it to the takeover, perform, then sell the car's parts at the chop shop. Cops, heat, the raid timer, the getaway chase, the dump, saving, and the HUD polish come later. This is a proof of concept: it should run in `L_Greybox` and be honest about what is placeholder.

## Goals

- One loop runs through `UMissionSubsystem`: BreakIn → Getaway (drive to the arena, no cops) → Takeover → Chop → Complete.
- Break-in: walk on foot, find the keys at one of several marked spots (chosen at random), get into the car.
- Takeover: starts when the car enters the crowd ring, lasts a fixed, tunable time, and converts the score to rep when it ends.
- Chop: when the takeover ends, a chop menu opens straight away (no drive to a garage). Each part takes a short time to remove and pays its value. Finishing the menu ends the loop.
- Money and rep accumulate across loops in the game instance (not saved yet).
- Pure logic is tested headless: loop phase order, chop removal and payout, and whether a point is inside the crowd ring.

## Non-goals

Heat, police, the raid timer, failure and partial rewards, the dump, saving to disk, the homeowner AI, noise, the removal timing minigame, hiding removed parts on the car mesh, the minimap, and UMG polish.

## Anti-goals

- Shortcuts that would have to be torn out rather than built on. The phase list, the phase enum, and where state lives follow `PLAN.md` section 4, so heat and the other phases slot in later.
- Hiding placeholder work. Anything stubbed is named in the code and in this document.

## Decisions

### Phase handling inside the subsystem, not phase objects (Claude's call)

- Chosen: `UMissionSubsystem` runs a `switch` over `EMissionPhase`, with the phase order in a plain `FMissionFlow` struct that tests drive directly.
- Alternatives: one `UMissionPhase` subclass per phase, as `PLAN.md` section 4 and ADR 0004 describe.
- Why: each POC phase is a few lines (poll a condition, or count down a timer). Phase classes would add a base class, five subclasses, and config wiring before any phase has real logic. The flow struct keeps the order testable either way, and phases move into classes when one grows real behavior.
- Wrong later if: a phase needs its own state and tick logic (break-in AI, pursuit). That is the trigger to introduce `UMissionPhase`, keeping `FMissionFlow`.

### Getaway kept as "drive to the arena" (Claude's call)

- Chosen: Getaway stays in the order with no cops. It completes when the car enters the crowd ring.
- Alternatives: start the takeover timer as soon as the car is stolen; drop Getaway from the order.
- Why: the takeover timer must not run during the drive, and keeping the phase means police slot into it later without reordering the loop.
- Wrong later if: the design moves the chase to after the takeover (Rashun raised a post-raid escape on 2026-11-01). Then Getaway becomes a plain drive and a new escape phase follows a raid.

### Takeover ends on a timer (Claude's call)

- Chosen: `TakeoverDurationSeconds` on `UMissionConfig`, default 90, counted from entering the ring.
- Alternatives: the player ends it with an input; the drift event ends it (M5); the crowd loses interest.
- Why: no new input assets, and a fixed length is easy to tune. The drift event replaces it in M5.
- Wrong later if: playtests show players want to leave early or stay longer. Then add a "leave" action.

### Rep from score (Claude's call)

- Chosen: rep = floor(score x `RepPerScorePoint`), default 0.01 (1 rep per 100 points).
- Why: the conversion isn't specified anywhere. A single tunable factor is the smallest thing that works.

### Chop menu built in C++ (Claude's call)

- Chosen: `UTakeoverChopMenuWidget` builds its UMG tree in code (a list of part buttons, a progress line, a payout total, a Finish button), driven by mouse or gamepad focus.
- Alternatives: a Widget Blueprint (`WBP_`) made over MCP; a Canvas menu on the debug HUD with keyboard keys.
- Why: it needs no editor assets, so the loop runs as soon as the C++ builds. The controller exposes the widget class as a property, so a designed Widget Blueprint can replace it later without code changes.
- Wrong later if: the menu gets real art or animation. Then make a Widget Blueprint subclass.

### Parts come from a DataTable

- Chosen: `FTakeoverChopPartRow` (display name, base value, removal seconds) in a DataTable on `UMissionConfig`, as `PLAN.md` section 5.4 says. Removal is a short wait, without the timing minigame.
- Source values for the table are in `Content/Takeover/Data/Source/ChopParts.csv`, for import on Windows.

### Key spots and the car door use one interactable component (Claude's call)

- Chosen: `UTakeoverInteractableComponent` (a scene component with a prompt and an `OnInteract` event). The character interacts with the nearest enabled one within reach. Key spots and the car each own one.
- Alternatives: overlap triggers that act automatically; a Blueprint interface.
- Why: the same component serves doors, drawers, and key hooks in M3, and the prompt drives the HUD hint.

### Keys at a random spot

- Chosen: on entering BreakIn, the subsystem picks one `ATakeoverKeySpot` with an `FRandomStream`. `KeySeed` on `UMissionConfig` (0 = random) makes a run repeatable. With no key spots placed, the player gets the keys at once, so the loop still runs.

## What exists after this branch

C++, unbuilt until the Windows machine compiles it:

- `Mission/`: `EMissionPhase`, `FMissionFlow`, `UMissionConfig`, `UMissionSubsystem`, debug console commands.
- `Core/TakeoverProgressSubsystem`: money, rep, loops completed (in memory).
- `Player/TakeoverCharacter`: third-person on-foot pawn with move, look, and interact.
- `Stealth/TakeoverInteractableComponent`, `Stealth/TakeoverKeySpot`.
- `Vehicle/TakeoverVehiclePawn`: a driver-door interactable and parking for the chop.
- `Arena/TakeoverRingGeometry`, `ATakeoverArena::IsInsideCrowdLine`.
- `Chop/TakeoverChopTypes`, `Chop/TakeoverChopSession`.
- `UI/TakeoverChopMenuWidget`; the debug HUD shows phase, objective, timer, prompt, money, and rep.
- Tests: `Takeover.Mission.Flow`, `Takeover.Chop.Session`, `Takeover.Arena.Ring`.

Editor work on Windows (listed in `docs/windows-checklist.md`): on-foot Input Actions and `IMC_OnFoot`, `DA_InputConfig` fields, `BP_TakeoverCharacter`, `DA_MissionConfig`, `DT_ChopParts`, a greybox house with key spots in `L_Greybox`, the hero car parked at the house, and the game mode's default pawn.

## Left to the builder

Defaults for reach distance, removal times, and camera values, all exposed as `UPROPERTY(EditDefaultsOnly)`. Debug commands for skipping phases.

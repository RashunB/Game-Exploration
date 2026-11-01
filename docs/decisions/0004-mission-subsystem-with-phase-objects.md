# 0004: `UMissionSubsystem` as a tickable world subsystem wiring phase objects

- Status: accepted, implemented in M2
- Date: 2026-10-23

## Context

The loop runs Idle, BreakIn, Getaway, Takeover, Chop, Dump, then Complete or Failed. Something has to own the current loop, drive transitions, and tick the active phase. In Unreal that owner could be an actor placed in the level, the GameMode, or a subsystem: an engine-managed singleton scoped to a world or the game instance.

## Decision

`UMissionSubsystem` is a `UTickableWorldSubsystem`, so it ticks without a host actor. Each phase is a `UMissionPhase` subclass with `Enter()`, `Tick()`, `Exit()`, and `OnPhaseComplete` / `OnPhaseFailed` delegates. Phases do not reference each other; the subsystem wires them. Phase order and classes come from a `UMissionConfig` DataAsset, and `FMissionLoopState` carries what phases need from each other.

## Alternatives

- A manager actor in the level: needs placing in every map and can be deleted by accident.
- Logic in the GameMode: GameMode is a framework class with its own lifecycle and is reached through the world anyway; putting the loop there mixes match rules with mission flow.
- Phases that call the next phase directly: simpler at first, but couples every phase to its neighbors and makes reordering or stubbing phases hard.

## Consequences

Phases can be stubbed (M2 completes every phase but Takeover on a debug command) and tested one transition at a time. The subsystem lives as long as the world, which, with no level travel (ADR 0003), means for the whole loop.

## Revisit if

Phases need to run in parallel or nest, which a single active-phase machine doesn't model.

## Sources

`PLAN.md` section 4 (Mission spine) states "so phases tick without a host actor" and "Phases do not reference each other. The subsystem wires them together." The alternatives are reconstructed.

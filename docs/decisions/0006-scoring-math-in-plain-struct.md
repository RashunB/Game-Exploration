# 0006: Scoring math in a plain struct for headless tests

- Status: accepted
- Date: 2026-10-23

## Context

The takeover scoring rules (slip angle, state priority, proximity, combo) are the core of the game and will be retuned often. Code in actors and components only runs inside a world, so testing it means spawning a world and a car, which is slow and needs the engine with a renderer.

## Decision

`FTakeoverScoringModel` is a plain C++ struct with no world access. `UTakeoverScoringComponent` builds an `FTakeoverVehicleSample` from the Chaos vehicle each tick and feeds it to the model. Tuning values live in a `USTRUCT` on the `UTakeoverScoringConfig` DataAsset. The same pattern is the rule for all testable logic (`AGENTS.md`).

## Alternatives

- Rules inside the component, tested with functional tests in a spawned world: slow, flaky, and needs a car to be physically put into a drift.
- No tests, verified by playing: regressions while tuning would go unnoticed.

## Consequences

23 automation tests (`Takeover.Scoring.Model`) run headless with `-nullrhi` in seconds. Collision filtering stays in the component, since judging an impulse needs the physics hit, so that part is not covered by the spec.

## Revisit if

Scoring starts needing world queries mid-calculation (line traces, multiple actors), which a single sample struct can't carry.

## Sources

`PLAN.md` sections 3 (Testing) and 5.3, and `Source/Takeover/Arena/TakeoverScoringModel.h`. Learning note [0002](../learning/notes/0002-scoring-model.md).

# 0011: Drift event over race for the takeover finale

- Status: accepted
- Date: 2026-10-24

## Context

The takeover phase ends in an event: either a drift event (score attack along a marked line) or a sprint race against simple AI. The choice was left to the M1 gate.

## Decision

Drift event.

## Alternatives

- Sprint race: needs AI drivers on Chaos, the same hard problem as police pursuit AI (`PLAN.md` section 5.2 calls it the largest unknown in the plan), and probably a paid asset such as Drift AI, whose listings don't show 5.8 support.

## Consequences

The finale reuses `FTakeoverScoringModel`, so M5 builds a marked line and a score attack around existing, tested scoring. AI driving risk stays confined to police pursuit in M4.

## Revisit if

Playtests show the drift event doesn't land as a finale, and pursuit AI from M4 turns out good enough to race against.

## Sources

`PLAN.md` section 5.3 (Event) and section 11 (Resolved), commit `f2fd58b`. The reason given is the scoring reuse versus the AI-driver problem; Rashun's own reasons at the gate are not recorded beyond the choice.

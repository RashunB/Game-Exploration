# 0012: Failure keeps banked rep and a fraction of unbanked money

- Status: accepted, implemented in M2
- Date: 2026-10-24

## Context

A loop can fail (arrested, car destroyed). The game needs a rule for what the player keeps.

## Decision

Rep already banked from the takeover is kept. Unbanked money is multiplied by `FailureRewardFraction` on `UMissionConfig`, default 0.25.

## Alternatives

Not recorded.

## Consequences

Failing late in a loop still pays something, so the takeover isn't wasted by a bad dump. The fraction is a tunable on a DataAsset, so balance is Rashun's playtest call. Reward math is covered by tests in M2.

## Revisit if

Playtests show players farm failures, or failing feels too punishing to retry.

## Sources

`PLAN.md` sections 4 (Mission spine) and 11 (Resolved), commit `f2fd58b`. Why 0.25, and why rep is kept while money is scaled, is not written down.

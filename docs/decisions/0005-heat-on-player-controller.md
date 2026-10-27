# 0005: Heat on the player controller

- Status: accepted, implemented in M2
- Date: 2026-10-23

## Context

Heat (the wanted level) has to persist across a loop while the player switches bodies: on foot in the break-in, in the car for the getaway and takeover. In Unreal the player controller represents the player and persists, while pawns (the character, the car) are bodies it possesses and can be swapped or destroyed.

## Decision

`UHeatComponent` lives on `ATakeoverPlayerController`, which also swaps possession between the on-foot character and the vehicle.

## Alternatives

- On the pawn: heat would be lost or need copying at every pawn swap.
- On the mission subsystem: workable, but heat is per player and the controller is the framework's per-player object.
- On PlayerState: the usual place in multiplayer games, for replication. This game is single-player, so that buys nothing.

## Consequences

Heat survives pawn swaps for free. Systems that add heat (crowd hits on the scoring component, alarms) need a path to the controller.

## Revisit if

Heat needs to outlive the controller, for example across a level travel, which ADR 0003 rules out.

## Sources

`PLAN.md` section 4 (Player and input, Heat): "Heat lives here so it survives pawn swaps." The alternatives are reconstructed.

# 0010: The hero car gets its own wheel classes

- Status: accepted
- Date: 2026-10-23

## Context

Chaos vehicles configure each wheel through a wheel class (a Blueprint subclass of `UChaosVehicleWheel`) whose class defaults hold grip, slip thresholds, handbrake torque, and steering angle. Epic issue UE-274631 reports per-car wheel configuration writing to the shared wheel class defaults, which would leak handling changes between every car using that class.

## Decision

The hero car uses its own `BP_HeroCar_WheelFront` and `BP_HeroCar_WheelRear`, not shared with police or traffic cars.

## Alternatives

- Share the template's wheel classes: fewer assets, but drift tuning on the hero car could change police handling, or the reverse.

## Consequences

Each future car type gets its own wheel classes. Wheel-class values are read when Chaos creates the vehicle, so changing them at runtime means recreating the vehicle's physics state, unlike the `UTakeoverHandlingProfile` values applied through runtime setters (`PLAN.md` section 4, Vehicle). Most of the M1 drift tuning happened in these two assets (see `docs/playtests/2026-10-24-m1-gate.md`).

## Revisit if

Epic fixes UE-274631 and per-instance wheel configuration becomes reliable.

## Sources

`PLAN.md` section 4 (Vehicle) and the 2026-10-23 revision history entry, commit `e34f7e2`. The issue number is from the plan; its status was not rechecked.

# 0002: C++ first, Blueprints as thin data subclasses

- Status: accepted
- Date: 2026-10-23

## Context

Unreal gameplay can be written in C++ or in Blueprints, its visual scripting. Blueprints are binary `.uasset` files. Claude works on text: it can read, diff, review, and refactor C++, but can change a Blueprint only through the editor over MCP, and nobody can review a Blueprint change in a pull request.

## Decision

All gameplay logic lives in C++. Blueprints are thin subclasses of C++ classes that hold tuning values and asset references only (for example `BP_HeroCar` subclassing the `Abstract` `ATakeoverVehiclePawn`). Tuning values are `UPROPERTY(EditDefaultsOnly)` or live in DataAssets.

## Alternatives

- Blueprint-first: faster iteration in the editor and the most common style in tutorials, but logic would be invisible to diffs and to Claude's text tools.
- Mixed, with logic wherever it is quickest: the same review problem for whatever ends up in Blueprints.

## Consequences

Every change to logic is reviewable as text and testable from C++. Adding a tunable needs a C++ change and a full build (a new `UPROPERTY` changes the class layout, which Live Coding can't patch). The code name `Takeover` is fixed, because renaming a module later needs CoreRedirects for every Blueprint subclass.

## Revisit if

A system turns out to be mostly content wiring (cinematics, UI layout) where Blueprint or UMG Designer work is clearly cheaper and the logic is trivial.

## Sources

`PLAN.md` section 2.

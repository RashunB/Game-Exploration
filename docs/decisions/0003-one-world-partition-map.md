# 0003: One persistent World Partition map with One File Per Actor

- Status: accepted
- Date: 2026-10-23

## Context

The loop runs through five phases in different places: house, streets, arena, chop shop, drop zones. Unreal can split these into separate levels and travel between them, or keep one world. Level travel tears down the world and everything that lives in it.

## Decision

One persistent World Partition map holds the whole loop, with no level travel during a loop. Maps use One File Per Actor (OFPA), so each placed actor is saved as its own small file under `Content/__ExternalActors__/` instead of inside one large `.umap`.

## Alternatives

- One level per phase with level travel: smaller levels, but world-scoped systems (the mission subsystem, heat) would lose their state at every transition and need to be rebuilt from a save or the game instance.
- Classic sub-level streaming without World Partition: possible, but OFPA comes with World Partition, and the single big `.umap` would be rewritten in LFS on every edit.

## Consequences

World subsystems keep their state across phases. Editing the level changes a few per-actor files, which keeps LFS storage growth down and makes level diffs readable at the file level. City Sample levels are not OFPA, so their Level Instance versions only render as streaming levels, which is why packed (`BPP_`) buildings are used instead (`CHANGELOG.md`).

## Revisit if

The map grows past what one World Partition world streams comfortably on the target hardware, or a phase needs a genuinely separate space (an interior with different lighting) that streaming can't handle.

## Sources

`PLAN.md` sections 3 (Version control) and 4 (World and lifetime). The OFPA reason (small per-actor files instead of rewriting one `.umap`) is stated there; the alternatives are reconstructed.

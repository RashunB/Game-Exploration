# 0007: Git LFS without file locking

- Status: accepted
- Date: 2026-10-23

## Context

Unreal assets are binary and large, so they go through Git LFS (`.gitattributes`). LFS can also lock files, the usual answer to binary files that can't be merged.

## Decision

Use Git LFS for `.uasset`, `.umap`, and binary media, without file locking.

## Alternatives

- LFS with locking: protects against two people editing one asset, but lockable files stay read-only until locked, which fights the editor's auto-save. There is one developer.
- Plain git: every binary version would bloat the repo history permanently.
- Perforce, the Unreal industry default: built for large teams and binaries, but adds a second VCS and server for a solo project already on GitHub.

## Consequences

No lock friction. The LFS budget matters: GitHub Free includes 10 GiB storage and 10 GiB bandwidth per month, and every re-save of a binary stores a full new copy. CI checks out without LFS objects to save bandwidth.

## Revisit if

A second person starts editing assets, or LFS storage nears the free budget.

## Sources

`PLAN.md` section 3 (Version control). The Perforce alternative is not in the record.

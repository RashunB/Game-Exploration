# 0009: Third-party content out of the public repo, restored from a hashed manifest

- Status: accepted
- Date: 2026-10-24

## Context

The repo is public. City Sample and Fab content is licensed for use in Unreal Engine products, not for publishing as source files, and the vehicle template ships with the engine. The M1 art pass migrated 1068 third-party files (5.09 GiB) into `Content/`. One commit of them to a public repo would publish licensed assets.

## Decision

Third-party content is never committed. `.gitignore` tracks only `Content/Takeover/`, `Content/__ExternalActors__/`, and `Content/__ExternalObjects__/`. `Tools/VendorContent/manifest.json` (committed) lists every third-party file with its source and SHA-256. `vendor_content.py restore` rebuilds them on a fresh clone from the local staging project and the engine install, `verify` checks them, and `record` updates the manifest after a migration. Third-party files are never edited, so the hashes stay valid. A pre-commit hook and CI both block a commit that stages third-party files.

## Alternatives

- Make the repo private: would allow committing the content, but loses the public repo, and the 5 GiB would exhaust the LFS budget.
- A private second repo or submodule for third-party content: still redistributes it, and doubles the LFS cost.
- Commit nothing and restore by hand: no reproducibility, and no way to tell which version of a pack a level depends on.

## Consequences

A fresh clone needs the staging project and the same pack versions to restore; the staging project and the Fab library are the only copies, so the staging project must be backed up (`PLAN.md` section 3). `restore` refuses a source file whose hash changed, so a pack update shows up as a verify failure rather than as silent drift.

## Revisit if

The repo goes private, or Epic's or Fab's license terms change.

## Sources

`PLAN.md` sections 3 and 7, commit `dab4dd3`, `CHANGELOG.md` (Changed). The history rewrite happened before the first push.

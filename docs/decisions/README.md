# Decision records

One file per significant decision, `NNNN-title.md`, from [TEMPLATE.md](TEMPLATE.md). Numbers are never reused. A changed decision gets a new record that supersedes the old one, and the old one's status says so. `PLAN.md` links each record from the section it shapes. Records 0001 to 0013 were written on 2026-10-27 from `PLAN.md`, `CHANGELOG.md`, and commit history; where the original reason was never written down, the record says so.

| ADR | Decision | Status |
| --- | --- | --- |
| [0001](0001-ue-5-8-over-5-7.md) | Unreal Engine 5.8 over 5.7 | accepted |
| [0002](0002-cpp-first-thin-blueprints.md) | C++ first, Blueprints as thin data subclasses | accepted |
| [0003](0003-one-world-partition-map.md) | One persistent World Partition map with One File Per Actor | accepted |
| [0004](0004-mission-subsystem-with-phase-objects.md) | `UMissionSubsystem` as a tickable world subsystem wiring phase objects | accepted, implemented in M2 |
| [0005](0005-heat-on-player-controller.md) | Heat on the player controller | accepted, implemented in M2 |
| [0006](0006-scoring-math-in-plain-struct.md) | Scoring math in a plain struct for headless tests | accepted |
| [0007](0007-git-lfs-without-locking.md) | Git LFS without file locking | accepted |
| [0008](0008-fictional-cars-and-brands.md) | Fictional cars and brands only | accepted |
| [0009](0009-third-party-content-out-of-repo.md) | Third-party content out of the public repo, restored from a hashed manifest | accepted |
| [0010](0010-hero-car-own-wheel-classes.md) | The hero car gets its own wheel classes | accepted |
| [0011](0011-drift-event-finale.md) | Drift event over race for the takeover finale | accepted |
| [0012](0012-failure-rewards.md) | Failure keeps banked rep and a fraction of unbanked money | accepted, implemented in M2 |
| [0013](0013-enable-livecodingtoolset.md) | Enable `LiveCodingToolset` separately | accepted |
| [0014](0014-agent-plugin-and-skill-stack.md) | Agent plugin and skill stack, with the no-worktrees override | accepted |

# Windows checklist

Work that only the Windows dev machine can do or verify: builds, the editor, Unreal MCP, tests, and anything Claude Code has to observe running locally. Cloud sessions add items here instead of marking them done. Tick an item when it is verified, note anything that failed, and delete a section once all of it is ticked.

## Agent tooling (added 2026-10-27, branch `chore/agent-docs-and-admin`)

Run these after pulling the branch, in Git Bash from the repo root.

- [ ] Hooks path: `sh Tools/githooks/install.sh`, then `git config core.hooksPath` prints `Tools/githooks`. (The Claude Code SessionStart hook also sets it when unset.)
- [ ] LFS still uploads: commit a real asset change (or the next one that comes along) and confirm `git push` prints the LFS upload line. That proves the LFS `pre-push` hook in `Tools/githooks/` runs.
- [ ] Git hook blocks third-party content: by hand, `mkdir -p Content/Foo && echo x > Content/Foo/a.txt && git add -f Content/Foo/a.txt && git commit -m test`. Expect "Blocked". Then `git restore --staged Content/Foo/a.txt && rm -r Content/Foo`.
- [ ] Claude Code loads both plugins: `/plugin` shows `unreal-engine-skills-for-claude-code` and `superpowers` enabled. `enabledPlugins` does not install anything, so if Superpowers is missing, run `/plugin install superpowers@claude-plugins-official`. Confirm the versions match `docs/tooling.md` (Superpowers 7.0.0 at `bb92a77`, Epic at `a6aa73a`).
- [ ] Superpowers' SessionStart hook runs under Git Bash without errors (no hook error on session start).
- [ ] Skills appear: the 26 `ue-*` skills, plus `/walkthrough`, `/learning-note`, `/your-turn`, `/quiz`, `/retro`, `/mcp-checkpoint`.
- [ ] Import works: in a fresh session, ask Claude for the test-first list without reading files. It should list save data round-trips, which only `AGENTS.md` mentions. (Already passed in a cloud session on 2026-10-27; recheck locally.)
- [ ] Commit guard fires under Git Bash: ask Claude to commit with a force-added file under `Content/Foo/` staged. Expect the PreToolUse hook to block it. Clean up as above.
- [ ] MCP: with the editor open, `/mcp` shows `unreal-mcp` connected. Read the exact tool names and confirm they match the hook matcher `mcp__.*unreal-mcp__.*` in `.claude/settings.json`. With an uncommitted change under `Config/`, the first editor call in a session shows the checkpoint warning, and later calls don't.
- [ ] Tutor guard on Windows paths: run `/learning-note 0003-scoring-component-and-config`. The note lands in `docs/learning/notes/`, and the session shows no blocked writes inside `docs/learning/`.
- [ ] Fresh clone: clone into a new folder, `sh Tools/githooks/install.sh`, `python Tools/VendorContent/vendor_content.py restore`, build the editor target, open the editor, and confirm `L_Greybox` loads and the `Takeover.` tests pass (24 at last count). Delete the extra clone afterwards; it has its own multi-gigabyte build output.

## Core loop POC (added 2026-11-01, branch `poc/core-loop`)

The C++ was written in a cloud session and has never been compiled. Design: `docs/superpowers/specs/2026-11-01-core-loop-poc-design.md`.

- [ ] Full build with the editor closed (new classes and properties; Live Coding can't do it). Fix compile errors before anything else.
- [ ] Headless tests: `Takeover.Mission.Flow` (10), `Takeover.Chop.Session` (8), and `Takeover.Arena.Ring` (4) pass with the existing 24.
- [ ] Input, through MCP in `Content/Takeover/Input/`: `IA_Move` and `IA_Look` (Axis2D) and `IA_Interact` (Digital), plus `IMC_OnFoot`. Move: WASD (Swizzle on W/S, Negate on A and S) and the gamepad left stick. Look: mouse XY and the right stick (Negate Y if pitch feels inverted). Interact: E and Gamepad Face Button Left. Set `OnFootContext`, `MoveAction`, `LookAction`, and `InteractAction` on `DA_InputConfig`.
- [ ] `BP_TakeoverCharacter` (parent `ATakeoverCharacter`) in `Content/Takeover/Blueprints/`. A capsule with no mesh is fine for the POC. Using the Third Person template mannequin makes it third-party content: migrate and `record` it through `Tools/VendorContent`.
- [ ] `DT_ChopParts` in `Content/Takeover/Data/`, row struct `TakeoverChopPartRow`, imported from `Tools/Data/ChopParts.csv`.
- [ ] `DA_MissionConfig` (`UMissionConfig`) with `ChopParts` set to `DT_ChopParts`. On `BP_TakeoverGameMode`, set `MissionConfig` and make `BP_TakeoverCharacter` the default pawn.
- [ ] `L_Greybox`: a greybox house from engine shapes with a `PlayerStart` inside, three to five `ATakeoverKeySpot` actors (give each a `SpotMesh`, such as a cube drawer), and `BP_HeroCar` parked outside. On `BP_HeroCar`, move the `DriverDoor` component to the driver's side.
- [ ] Play the loop in PIE: search spots, find the keys, get in, drive into the crowd ring, perform until the timer ends, strip parts in the chop menu (mouse and gamepad), sell and finish. Money and rep show top right. `takeover.Mission.Skip` and `takeover.Mission.Restart` are the debug commands. Log the run in `docs/playtests/`.

## GitHub (needs `gh auth status` to pass; any machine)

The cloud session's proxy refuses repository-settings writes, and branch deletions failed from it. Labels, milestones, and issues worked.

- [x] Labels, milestones (M0 and M1 closed), M2 issues #1 to #9, tooling issue #10, and learning-note issues #11 to #15. Created from the cloud session on 2026-11-01 with `python .github/scripts/bootstrap_github.py`; rerunning it changes nothing.
- [ ] Default branch and retired branches: `python .github/scripts/bootstrap_github.py branches`. Sets `main` as default, then deletes `claude/plan-refinement-setup-9mfg9o` and `claude/m1-takeover-core` if `main` contains them.
- [ ] Protect `main`: `python .github/scripts/bootstrap_github.py protect` (pull requests required, `repo-checks` must pass).

## Docs for Rashun to confirm

- [ ] Correct `docs/playtests/2026-10-24-m1-gate.md`, which was reconstructed from commits, and fill its [unknown] gaps.
- [ ] Confirm or correct the reason in `docs/decisions/0008-fictional-cars-and-brands.md`.

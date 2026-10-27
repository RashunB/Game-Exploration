# Windows checklist

Work that only the Windows dev machine can do or verify: builds, the editor, Unreal MCP, tests, and anything Claude Code has to observe running locally. Cloud sessions add items here instead of marking them done. Tick an item when it is verified, note anything that failed, and delete a section once all of it is ticked.

## Agent tooling (added 2026-10-27, branch `chore/agent-docs-and-admin`)

Run these after pulling the branch, in Git Bash from the repo root.

- [ ] Hooks path: `sh Tools/githooks/install.sh`, then `git config core.hooksPath` prints `Tools/githooks`. (The Claude Code SessionStart hook also sets it when unset.)
- [ ] LFS still uploads: commit a real asset change (or the next one that comes along) and confirm `git push` prints the LFS upload line. That proves the LFS `pre-push` hook in `Tools/githooks/` runs.
- [ ] Git hook blocks third-party content: by hand, `mkdir -p Content/Foo && echo x > Content/Foo/a.txt && git add -f Content/Foo/a.txt && git commit -m test`. Expect "Blocked". Then `git restore --staged Content/Foo/a.txt && rm -r Content/Foo`.
- [ ] Claude Code loads both plugins: `/plugin` shows `unreal-engine-skills-for-claude-code` and `superpowers` enabled. Record both versions in `docs/tooling.md`.
- [ ] Superpowers' SessionStart hook runs under Git Bash without errors (no hook error on session start).
- [ ] Skills appear: the 26 `ue-*` skills, plus `/walkthrough`, `/learning-note`, `/your-turn`, `/quiz`, `/retro`, `/mcp-checkpoint`.
- [ ] Import works: in a fresh session, ask Claude for the test-first list without reading files. It should list save data round-trips, which only `AGENTS.md` mentions. (Already passed in a cloud session on 2026-10-27; recheck locally.)
- [ ] Commit guard fires under Git Bash: ask Claude to commit with a force-added file under `Content/Foo/` staged. Expect the PreToolUse hook to block it. Clean up as above.
- [ ] MCP: with the editor open, `/mcp` shows `unreal-mcp` connected. Read the exact tool names and confirm they match the hook matcher `mcp__.*unreal-mcp__.*` in `.claude/settings.json`. With an uncommitted change under `Config/`, the first editor call in a session shows the checkpoint warning, and later calls don't.
- [ ] Tutor guard on Windows paths: run `/learning-note 0003-scoring-component-and-config`. The note lands in `docs/learning/notes/`, and the session shows no blocked writes inside `docs/learning/`.
- [ ] Fresh clone: clone into a new folder, `sh Tools/githooks/install.sh`, `python Tools/VendorContent/vendor_content.py restore`, build the editor target, open the editor, and confirm `L_Greybox` loads and the `Takeover.` tests pass (24 at last count). Delete the extra clone afterwards; it has its own multi-gigabyte build output.

## GitHub (needs `gh auth status` to pass; any machine)

The cloud session's proxy refuses repository-settings writes, and tag pushes and branch deletions failed from it.

- [ ] Labels, milestones (M0 and M1 closed), M2 issues, and learning-note issues: `python .github/scripts/bootstrap_github.py`. Try `--dry-run` first.
- [ ] Default branch and retired branches: `python .github/scripts/bootstrap_github.py branches`. Sets `main` as default, then deletes `claude/plan-refinement-setup-9mfg9o` and `claude/m1-takeover-core` if `main` contains them.
- [ ] Protect `main`: `python .github/scripts/bootstrap_github.py protect` (pull requests required, `repo-checks` must pass).
- [ ] M1 gate tag: `git tag -a m1-gate dab4dd3 -m "M1 gate passed (2026-10-24)" && git push origin m1-gate`.

## Docs for Rashun to confirm

- [ ] Correct `docs/playtests/2026-10-24-m1-gate.md`, which was reconstructed from commits, and fill its [unknown] gaps.
- [ ] Confirm or correct the reason in `docs/decisions/0008-fictional-cars-and-brands.md`.

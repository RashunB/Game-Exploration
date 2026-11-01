"""Creates this repo's GitHub labels, milestones, and M2 issues, and sets up the main branch.

Cloud Claude Code sessions can't create labels or milestones or change repo settings, so this runs
on a machine where `gh auth status` passes. Every step is idempotent: existing labels are updated,
existing milestones and issues (matched by title) are left alone.

Usage (from the repo root; Python 3.9+, gh 2.x):
  bootstrap_github.py [--dry-run] [step ...]
Steps:
  labels       create or update the labels
  milestones   create M0 to M6 with their acceptance criteria; close M0 and M1
  issues       open the M2 issues and the learning-note issues
  branches     make main the default branch, then delete the retired claude/* branches if main has them
  protect      protect main: changes by pull request only, repo-checks must pass
With no steps, runs labels, milestones, and issues. branches and protect change repo settings, so
they only run when named.
"""
import json
import subprocess
import sys

REPO = "RashunB/Game-Exploration"
RETIRED_BRANCHES = ("claude/plan-refinement-setup-9mfg9o", "claude/m1-takeover-core")
REQUIRED_CHECK = "repo-checks"

LABELS = [(f"milestone:M{n}", "1d76db", f"Milestone M{n} (PLAN.md section 8)") for n in range(7)] + [
	("type:feature", "0e8a16", "New gameplay or tooling"),
	("type:bug", "d73a4a", "Something behaves wrong"),
	("type:chore", "c5def5", "Maintenance, setup, cleanup"),
	("type:docs", "0075ca", "Documentation"),
	("feel-gate", "fbca04", "Done only after Rashun's playtest, logged in docs/playtests/"),
	("owner:rashun", "5319e7", "Your-turn task: Rashun implements, Claude reviews"),
	("windows-only", "bfd4f2", "Needs the Windows dev machine (build, editor, MCP, tests)"),
	("ask-first", "e99695", "Needs Rashun's approval before work starts (AGENTS.md)"),
	("learning", "d4c5f9", "Learning material: notes, walkthroughs, retros"),
]

MILESTONES = [
	("M0: Setup", "closed", """Week 1. Project skeleton, toolchain, MCP, greybox map.

Acceptance:
- A fresh clone builds, the editor opens, and MCP responds to a read-only tool call.
- Takeover.Smoke passes from the command line.
- The greybox map loads.

Passed 2026-10-24."""),
	("M1: Takeover core", "closed", """Weeks 2 to 4. Vehicle pawn with the Drift profile, scoring model, component, and tests, placeholder crowd spline, debug feedback.

Acceptance (gate): Rashun playtests the greybox on gamepad and keyboard and confirms the takeover is fun.

Passed 2026-10-24. Drift event chosen for the finale. Tire smoke, skid marks, and the burnout check carry over into M2. See docs/playtests/2026-10-24-m1-gate.md."""),
	("M2: Mission spine, heat, HUD", "open", """Week 5. Mission subsystem, loop state, progress subsystem and save game, heat, raid timer, HUD.

Acceptance:
- A full loop runs in L_Greybox through UMissionSubsystem: Idle, BreakIn, Getaway, Takeover, Chop, Dump, Complete. Every phase but Takeover is a stub that completes on a debug command until M3 to M5. Phase transitions are covered by automation tests.
- Heat: 0 to 100 with 4 tiers and hysteresis. Crowd hits and takeover duration add heat, and heat decays while unseen (line-of-sight stub until M4). Tier changes, hysteresis, and decay are covered by tests.
- Raid timer: tier 3 during the takeover starts a countdown. Leaving the arena before it ends banks rep converted from score. Running out means arrest: the loop fails with partial rewards (banked rep kept, unbanked money x FailureRewardFraction, default 0.25). Reward math is covered by tests.
- Money, rep, and loops completed survive a PIE restart (saved on loop end and on quit).
- HUD: heat meter, score and combo, objective text, raid countdown, and a minimap (top-down scene capture around the car with a player arrow and an objective marker). The debug HUD moves behind a console variable.
- Heat rates and raid timing go to Rashun for a playtest. They are not marked done from code.
- Carry-overs from M1: tire smoke, skid marks, burnout check."""),
	("M3: Break-in", "open", """Weeks 6 to 7. Break-in phase, on-foot character, pawn swap into the car.

Acceptance (PLAN.md 5.1): the player can sneak in, find the keys, and leave. Getting spotted raises heat and does not hard-fail the mission."""),
	("M4: Getaway and dump", "open", """Weeks 8 to 9. Getaway pursuit and dump phase.

Acceptance (PLAN.md 5.2, 5.5):
- A pursuit can be escaped, and arrest triggers when the car is stopped and surrounded for a set time.
- Dropping the shell undetected completes the loop. Getting caught fails it with partial rewards."""),
	("M5: Chop and takeover event", "open", """Week 10. Chop menu and the drift event.

Acceptance (PLAN.md 5.3, 5.4):
- Money is awarded per part, part values are covered by automation tests, and the car switches to the Stripped profile.
- The drift event (score attack along a marked line) runs at the end of the takeover."""),
	("M6: Art, audio, packaging", "open", """Weeks 11 to 13. Art pass with City Sample assets, audio, settings menu, gamepad menu navigation, crash reporting, packaging (confirm no MCP plugin code ships), store assets.

Acceptance: a packaged Windows build that contains no MCP plugin code, with the art pass, audio, settings, and gamepad menus in place."""),
]

M2 = "M2: Mission spine, heat, HUD"

ISSUES = [
	("Mission loop through UMissionSubsystem with stub phases", M2, ["type:feature", "windows-only"], """`UMissionSubsystem` (tickable world subsystem) runs Idle, BreakIn, Getaway, Takeover, Chop, Dump, Complete in `L_Greybox`, with `UMissionPhase` subclasses wired by the subsystem and phase order from `UMissionConfig`. Every phase but Takeover is a stub that completes on a debug command. `FMissionLoopState` carries data between phases.

Acceptance:
- [ ] A full loop runs in PIE.
- [ ] Phase transitions, including Failed, are covered by `Takeover.` automation tests written first.

PLAN.md sections 4 and 8 (M2). ADR 0004."""),
	("Heat: tiers, hysteresis, decay, and heat events", M2, ["type:feature", "windows-only"], """`UHeatComponent` on `ATakeoverPlayerController`: 0 to 100, tiers 0 clean, 1 noticed, 2 pursued, 3 raid. Thresholds, hysteresis margin, and rates in a DataAsset. Crowd hits (`UTakeoverScoringComponent::OnCrowdHit`) and takeover duration add heat. Heat decays while unseen; the line-of-sight query is a stub that reports unseen until M4.

Acceptance:
- [ ] Tier changes, hysteresis, and decay are covered by tests written first, with the math in a plain struct.
- [ ] Rates are tunable without code changes.

The rates themselves are feel work: see the playtest issue. PLAN.md section 4 (Heat). ADR 0005."""),
	("Raid timer and failure rewards", M2, ["type:feature", "windows-only"], """Tier 3 during the takeover starts a countdown. Leaving the arena before it ends banks rep converted from score. Running out means arrest: the loop fails with partial rewards (banked rep kept, unbanked money x `FailureRewardFraction`, default 0.25, on `UMissionConfig`).

Acceptance:
- [ ] Reward math and the rep conversion are covered by tests written first.
- [ ] Arrest ends the loop as Failed with the failure reason in `FMissionLoopState`.

ADR 0012."""),
	("Progress subsystem and save game", M2, ["type:feature", "windows-only"], """`UTakeoverProgressSubsystem` (game instance subsystem) owns money, rep, and loops completed, and saves `UTakeoverSaveGame` on loop end and on quit.

Acceptance:
- [ ] Values survive a PIE restart.
- [ ] A save data round-trip test covers every field."""),
	("HUD: heat, score, objective, raid countdown, minimap", M2, ["type:feature", "windows-only"], """UMG HUD with C++ base classes in `Source/Takeover/UI/`: heat meter, score and combo, objective text, raid countdown, and a minimap (top-down scene capture around the car with a player arrow and an objective marker; M4 adds police blips). `ATakeoverDebugHUD` moves behind a console variable.

Acceptance:
- [ ] Every element updates from game state in PIE.
- [ ] The debug HUD is off by default and toggled by a console variable."""),
	("Playtest heat rates and raid timing", M2, ["feel-gate", "windows-only"], """Rashun playtests the M2 loop and tunes heat gain, decay, tier thresholds, and the raid timer. Closed by an entry in `docs/playtests/`, not by code.

Depends on the heat and raid timer issues."""),
	("Tire smoke (carry-over from M1)", M2, ["type:feature", "feel-gate", "windows-only"], """Niagara tire smoke on the hero car, driven by wheel slip and skid from Chaos (the values `takeover.Telemetry` already logs). Rashun judges the look in a playtest."""),
	("Skid marks (carry-over from M1)", M2, ["type:feature", "feel-gate", "windows-only"], """Skid marks on the ground while tires skid, driven by Chaos wheel state. Rashun judges the look in a playtest."""),
	("Burnout check (carry-over from M1)", M2, ["type:feature", "feel-gate", "windows-only"], """Scoring detects a burnout (throttle plus brake at low speed), but with the default wheel setup Chaos brakes every wheel, so the rears probably won't spin. Check in a drive. If they don't spin, brake only the front axle while both inputs are held.

PLAN.md section 8 (M1, "Not yet built")."""),
	("Verify the agent tooling on the Windows machine", None, ["type:chore", "windows-only"], """Work through `docs/windows-checklist.md` (agent tooling section): plugins load, hooks fire under Git Bash, `core.hooksPath` is set, skills appear, and a fresh clone plus `vendor_content.py restore` builds and opens `L_Greybox`."""),
] + [
	(f"Learning note {number}: {title}", M2, ["learning", "type:docs"], f"""Write `docs/learning/notes/{number}-{slug}.md` with `/learning-note`, following `docs/learning/notes/TEMPLATE.md`, and update the concept map and glossary. Include one your-turn task.

Covers: {covers}

Order and scope: `docs/learning/README.md` (learning backlog).""")
	for number, slug, title, covers in [
		("0003", "scoring-component-and-config", "scoring component and config", "how `UTakeoverScoringComponent` samples the Chaos vehicle each tick and feeds the model, collision filtering, crowd hits, and `UTakeoverScoringConfig` as a DataAsset."),
		("0004", "vehicle-pawn-and-handling", "vehicle pawn and handling", "`ATakeoverVehiclePawn`, `UTakeoverHandlingProfile`, which Chaos values apply at runtime and which live on wheel class defaults, the hero car's own wheel classes (UE-274631), the gearbox and rotation damping fixes from the playtest commits, and the telemetry cvar."),
		("0005", "input-and-controller", "input and controller", "Enhanced Input (Input Actions, Mapping Contexts, `UTakeoverInputConfig`), the controller swapping contexts per pawn, and game mode wiring."),
		("0006", "arena-and-greybox", "arena and greybox", "`ATakeoverArena` and its crowd spline, `L_Greybox` with World Partition and One File Per Actor, and what the `__ExternalActors__` files are."),
		("0007", "vendor-content-pipeline", "vendor content pipeline", "why third-party content stays out of a public repo, and how the manifest, dependency closure, restore, verify, and record work."),
	]
]


def gh(args, dry_run, data=None, read=False):
	"""Runs gh. In a dry run, read calls still run and write calls are printed."""
	if dry_run and not read:
		print("would run: gh " + " ".join(args) + (" <<< " + json.dumps(data) if data else ""))
		return None
	result = subprocess.run(["gh"] + args, input=json.dumps(data) if data else None, capture_output=True, text=True)
	if result.returncode != 0 and dry_run:
		print("could not read (dry run continues as if empty): gh " + " ".join(args))
		return None
	if result.returncode != 0:
		raise SystemExit("gh %s failed: %s" % (" ".join(args), result.stderr.strip()))
	return json.loads(result.stdout) if result.stdout.strip() else None


def api_list(path, dry_run):
	pages = gh(["api", "--paginate", "--slurp", path], dry_run, read=True) or []
	return [item for page in pages for item in page]


def labels(dry_run):
	existing = {label["name"] for label in api_list(f"repos/{REPO}/labels?per_page=100", dry_run)}
	for name, color, description in LABELS:
		args = ["label", "create", name, "--repo", REPO, "--color", color, "--description", description]
		if name in existing:
			args.append("--force")
		gh(args, dry_run)
	print("labels: %d defined" % len(LABELS))


def milestone_numbers(dry_run):
	return {m["title"]: m for m in api_list(f"repos/{REPO}/milestones?state=all&per_page=100", dry_run)}


def milestones(dry_run):
	existing = milestone_numbers(dry_run)
	for title, state, description in MILESTONES:
		if title not in existing:
			gh(["api", "-X", "POST", f"repos/{REPO}/milestones", "--input", "-"], dry_run,
				data={"title": title, "state": state, "description": description})
		elif existing[title]["state"] != state:
			gh(["api", "-X", "PATCH", f"repos/{REPO}/milestones/{existing[title]['number']}", "--input", "-"], dry_run,
				data={"state": state})
	print("milestones: %d defined" % len(MILESTONES))


def issues(dry_run):
	numbers = milestone_numbers(dry_run)
	titles = {issue["title"] for issue in api_list(f"repos/{REPO}/issues?state=all&per_page=100", dry_run)}
	created = 0
	for title, milestone, issue_labels, body in ISSUES:
		if title in titles:
			continue
		data = {"title": title, "body": body, "labels": issue_labels + (["milestone:M2"] if milestone == M2 else [])}
		if milestone:
			if milestone not in numbers and not dry_run:
				raise SystemExit("milestone %r does not exist yet; run the milestones step first" % milestone)
			data["milestone"] = numbers[milestone]["number"] if milestone in numbers else "<" + milestone + ">"
		gh(["api", "-X", "POST", f"repos/{REPO}/issues", "--input", "-"], dry_run, data=data)
		created += 1
	print("issues: %d created, %d already present" % (created, len(ISSUES) - created))


def branches(dry_run):
	gh(["repo", "edit", REPO, "--default-branch", "main"], dry_run)
	for branch in RETIRED_BRANCHES:
		try:
			compare = gh(["api", f"repos/{REPO}/compare/main...{branch}"], dry_run, read=True)
		except SystemExit:
			print("branch %s: not found, skipped" % branch)
			continue
		if compare is None:
			continue
		if compare["ahead_by"] != 0:
			print("branch %s: %d commits not on main, kept" % (branch, compare["ahead_by"]))
			continue
		gh(["api", "-X", "DELETE", f"repos/{REPO}/git/refs/heads/{branch}"], dry_run)
		print("branch %s: on main, deleted" % branch)


def protect(dry_run):
	gh(["api", "-X", "PUT", f"repos/{REPO}/branches/main/protection", "--input", "-"], dry_run, data={
		"required_status_checks": {"strict": False, "contexts": [REQUIRED_CHECK]},
		# Admins can still push in an emergency; everyday changes go through pull requests
		"enforce_admins": False,
		"required_pull_request_reviews": {"required_approving_review_count": 0},
		"restrictions": None,
		"allow_force_pushes": False,
		"allow_deletions": False,
	})
	print("main: pull requests required, %s must pass" % REQUIRED_CHECK)


STEPS = {"labels": labels, "milestones": milestones, "issues": issues, "branches": branches, "protect": protect}


def main(argv):
	dry_run = "--dry-run" in argv
	names = [arg for arg in argv if arg != "--dry-run"] or ["labels", "milestones", "issues"]
	unknown = [name for name in names if name not in STEPS]
	if unknown:
		raise SystemExit("unknown step(s): %s; choose from %s" % (", ".join(unknown), ", ".join(STEPS)))
	for name in names:
		STEPS[name](dry_run)


if __name__ == "__main__":
	main(sys.argv[1:])

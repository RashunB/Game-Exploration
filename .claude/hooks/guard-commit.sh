#!/bin/bash
# PreToolUse on Bash `git commit` and `git add`: keeps third-party Content out of this public repo.
# Blocks a commit whose index stages Content/ outside the first-party folders, and a force-add under
# Content/ (the only way to stage a git-ignored third-party file). Reminds about vendor_content.py
# verify when level files are staged. Tools/githooks/pre-commit enforces the same rule for every commit.
source "$(dirname "$0")/lib.sh"
cd "${CLAUDE_PROJECT_DIR:-.}" || exit 0

command=$(json_field command)

if printf '%s' "$command" | grep -q -E 'git([[:space:]]+-[^[:space:]]+)*[[:space:]]+add([[:space:]]|$)' \
	&& printf '%s' "$command" | grep -q -E '(^|[[:space:]])(-f|--force|-[a-zA-Z]*f[a-zA-Z]*)([[:space:]]|$)' \
	&& printf '%s' "$command" | grep -q 'Content'; then
	echo "Blocked: force-adding under Content/ would stage git-ignored third-party content in a public repo (PLAN.md section 3). Only Content/Takeover/ and the level's __ExternalActors__/__ExternalObjects__ are tracked, and they need no -f." >&2
	exit 2
fi

printf '%s' "$command" | grep -q -E 'git([[:space:]]+-[^[:space:]]+)*[[:space:]]+commit([[:space:]]|$)' || exit 0

if ! output=$(sh Tools/githooks/check-staged-content.sh 2>&1); then
	printf '%s\n' "$output" >&2
	exit 2
fi

if [ -n "$output" ]; then
	warn_json "Level files are staged. Run python Tools/VendorContent/vendor_content.py verify on the Windows machine before committing level changes, or confirm with Rashun that it already passed."
fi
exit 0

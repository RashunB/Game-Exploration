#!/bin/sh
# Points this clone's git hooks at Tools/githooks: the pre-commit Content guard plus Git LFS's own
# hooks. core.hooksPath replaces .git/hooks entirely, so the LFS hooks are kept here too; without
# pre-push, a push would send LFS pointers without their objects.

cd "$(git rev-parse --show-toplevel)" || exit 1

if ! command -v git-lfs >/dev/null 2>&1; then
	echo "git-lfs is not on PATH. Install Git LFS first (PLAN.md section 3)." >&2
	exit 1
fi

current=$(git config --local --get core.hooksPath)
if [ -n "$current" ] && [ "$current" != "Tools/githooks" ]; then
	echo "core.hooksPath is already set to '$current'. Leaving it alone; set it to Tools/githooks by hand if that is intended." >&2
	exit 1
fi

git config --local core.hooksPath Tools/githooks
echo "core.hooksPath = Tools/githooks"

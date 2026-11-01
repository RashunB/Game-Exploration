#!/bin/bash
# SessionStart: point git at Tools/githooks if nothing else is configured, so the pre-commit
# Content guard and the Git LFS hooks protect commits in every session, cloud ones included.
cd "${CLAUDE_PROJECT_DIR:-.}" || exit 0
command -v git-lfs >/dev/null 2>&1 || exit 0
# Any scope counts: a global hooksPath is someone's deliberate setup, so don't override it, but say
# that the repo's pre-commit Content guard is not active.
current=$(git config --get core.hooksPath)
case "$current" in
	Tools/githooks) ;;
	"") git config --local core.hooksPath Tools/githooks && echo "Set core.hooksPath to Tools/githooks (pre-commit Content guard and Git LFS hooks)." ;;
	*) echo "core.hooksPath is $current, so Tools/githooks/pre-commit (the third-party Content guard) does not run in this clone. Tell Rashun; sh Tools/githooks/install.sh sets it once he agrees." ;;
esac
exit 0

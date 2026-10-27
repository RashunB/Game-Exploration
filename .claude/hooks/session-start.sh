#!/bin/bash
# SessionStart: point git at Tools/githooks if nothing else is configured, so the pre-commit
# Content guard and the Git LFS hooks protect commits in every session, cloud ones included.
cd "${CLAUDE_PROJECT_DIR:-.}" || exit 0
command -v git-lfs >/dev/null 2>&1 || exit 0
[ -n "$(git config --local --get core.hooksPath)" ] && exit 0
git config --local core.hooksPath Tools/githooks && echo "Set core.hooksPath to Tools/githooks (pre-commit Content guard and Git LFS hooks)."
exit 0

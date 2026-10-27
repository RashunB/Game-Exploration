#!/bin/sh
# Fails when the index stages a file under Content/ outside this project's own folders.
# Third-party content (City Sample, Fab, engine templates) must never reach this public repo
# (PLAN.md section 3). Used by Tools/githooks/pre-commit and .claude/hooks/guard-commit.sh.
# Keep the folder list in sync with .gitignore and FIRST_PARTY in Tools/VendorContent/vendor_content.py.

cd "$(git rev-parse --show-toplevel)" || exit 1

# Additions, copies, modifications, renames, and type changes; deleting a stray file stays allowed
staged=$(git -c core.quotePath=false diff --cached --name-only --diff-filter=ACMRT -- Content/)
blocked=$(printf '%s\n' "$staged" | grep -v -E '^Content/(Takeover|__ExternalActors__|__ExternalObjects__)/' | grep -v '^$')

if [ -n "$blocked" ]; then
	echo "Blocked: staged files under Content/ outside Content/Takeover/, Content/__ExternalActors__/, and Content/__ExternalObjects__/:" >&2
	printf '%s\n' "$blocked" | sed 's/^/  /' >&2
	echo "Third-party content must not be committed to this public repo (PLAN.md section 3)." >&2
	echo "Unstage with: git restore --staged -- <path>" >&2
	exit 1
fi

# Level changes: remind, don't block. verify needs the Windows staging project.
if printf '%s\n' "$staged" | grep -q -E '^Content/(__ExternalActors__|__ExternalObjects__)/|\.umap$'; then
	echo "Level files are staged. Run 'python Tools/VendorContent/vendor_content.py verify' before committing level changes." >&2
fi

exit 0

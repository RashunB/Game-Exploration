#!/bin/bash
# PreToolUse on Unreal MCP tools: warns, once per session, when Content/ or Config/ has uncommitted
# changes before Claude drives the editor. Editor changes are easiest to undo from a clean commit.
# Never blocks.
source "$(dirname "$0")/lib.sh"
cd "${CLAUDE_PROJECT_DIR:-.}" || exit 0

session=$(json_field session_id | tr -cd 'A-Za-z0-9_-')
marker="$(git rev-parse --git-dir)/takeover-mcp-checkpoint-${session:-unknown}"
[ -e "$marker" ] && exit 0

if [ -n "$(git status --porcelain -- Content Config)" ]; then
	: > "$marker"
	warn_json "Content/ or Config/ has uncommitted changes and Claude is about to call the Unreal editor. Commit them first (/mcp-checkpoint) or ask Rashun, so an editor change that goes wrong can be reverted from git. This warning shows once per session."
fi
exit 0

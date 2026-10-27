#!/bin/bash
# PreToolUse inside the tutor subagent only (.claude/agents/tutor.md): the tutor may write under
# docs/learning/ and run read-only git commands. Everything else is blocked.
source "$(dirname "$0")/lib.sh"

input=$(cat)
tool=$(printf '%s' "$input" | json_field tool_name)

case "$tool" in
	Write|Edit)
		# JSON escapes Windows backslashes; normalize to forward slashes
		path=$(printf '%s' "$input" | json_field file_path | sed 's#\\\\#/#g')
		case "$path" in
			*..*) ;;
			docs/learning/*|*/docs/learning/*) exit 0 ;;
		esac
		echo "Blocked: the tutor writes only under docs/learning/ (got: $path)." >&2
		exit 2
		;;
	Bash)
		command=$(printf '%s' "$input" | json_field command)
		if printf '%s' "$command" | grep -q -E '^git (log|diff|show|blame|ls-files|ls-tree|rev-parse|merge-base|shortlog|cat-file|branch --list|status)([[:space:]]|$)' \
			&& ! printf '%s' "$command" | grep -q -E '[;&|<>`]|\$\(|--output'; then
			exit 0
		fi
		echo "Blocked: the tutor runs only single read-only git commands (log, diff, show, blame, ls-files, ls-tree, rev-parse, merge-base, shortlog, cat-file, status), with no pipes or redirects. Use Read, Grep, and Glob for files." >&2
		exit 2
		;;
esac
exit 0

# Shared helpers for the Claude Code hooks in this folder. Sourced, not run.
# Hooks run under Git Bash on Windows, which ships sed and grep but not jq or a Python on PATH,
# so hook input JSON is read with sed. Good enough for the flat string fields used here.

# Prints the string value of a top-level or nested JSON field: json_field <name> < input
json_field() {
	sed -n 's/.*"'"$1"'"[[:space:]]*:[[:space:]]*"\(\([^"\\]\|\\.\)*\)".*/\1/p' | head -n 1
}

# Prints a PreToolUse response that adds context for Claude and shows a notice to the user.
# The message must not contain double quotes or backslashes.
warn_json() {
	printf '{"systemMessage":"%s","hookSpecificOutput":{"hookEventName":"PreToolUse","additionalContext":"%s"}}\n' "$1" "$1"
}

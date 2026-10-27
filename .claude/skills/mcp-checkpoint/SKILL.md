---
name: mcp-checkpoint
description: Commit current work before an Unreal MCP session that touches several assets.
disable-model-invocation: true
---

Make a checkpoint commit so editor changes made over Unreal MCP can be reverted from git.

1. Ask Rashun to save all in the editor (File > Save All) if it is open, and wait for his reply. Unsaved editor changes are not on disk yet.
2. Run `git status --short`. If there is nothing to commit, say so and stop.
3. Show the list. Stage only first-party paths: `Content/Takeover/`, `Content/__ExternalActors__/`, `Content/__ExternalObjects__/`, `Config/`, `Source/`, and docs. Never force-add anything under `Content/`. If something outside those paths looks unexpected, ask before staging it.
4. If level files are staged (`.umap` or `__ExternalActors__`/`__ExternalObjects__`), run `python Tools/VendorContent/vendor_content.py verify` first when on the Windows machine. In a cloud session, say it still needs running.
5. Commit with the message `Checkpoint before MCP session: <what the session will do>`, ending with the session's attribution lines.
6. Report the commit hash. Do not push unless asked.

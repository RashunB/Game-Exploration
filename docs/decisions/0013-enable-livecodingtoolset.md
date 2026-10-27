# 0013: Enable `LiveCodingToolset` separately

- Status: accepted
- Date: 2026-10-24

## Context

`CLAUDE.md` tells Claude to compile through Live Coding over MCP (`LiveCodingToolset.CompileLiveCoding`) while the editor is open, because a full build needs the editor closed and drops the MCP connection. `AllToolsets` does not include `LiveCodingToolset`, and it is off by default, so the tool was not exposed.

## Decision

Enable the `LiveCodingToolset` plugin in `Takeover.uproject`, Editor target only.

## Alternatives

- Full builds only: every C++ change would close the editor and break the MCP session.
- Trigger Live Coding by hand in the editor: works, but Claude couldn't compile and continue on its own.

## Consequences

Function-body changes compile without closing the editor. Class layout changes still need a full build. It is one more Experimental, editor-only plugin.

## Revisit if

`AllToolsets` starts including it, or Epic ships an MCP proxy that survives full builds.

## Sources

Commit `9667140`, `PLAN.md` section 3 (Editor access) and the 2026-10-24 revision history entry.

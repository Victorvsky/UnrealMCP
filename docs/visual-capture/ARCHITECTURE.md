# Visual capture: how it fits UnrealMCP

Phase 0 orientation, 2026-09-14. Facts below were read from the code; line numbers refer to
`Source/UnrealMCP/Private/MCPTcpServer.cpp` (C++) and `Python/src/unreal_mcp/` (server).

## What exists today

**Transport.** The plugin runs a loopback TCP listener on an OS-assigned port and writes it to
`Saved/UnrealMCP/port.txt` (`:478`). The Python MCP server (low-level `mcp.server.Server`, stdio,
`main.py`) finds the file through `UE_PROJECT_PATH`, opens one persistent connection, and sends
newline-delimited JSON: `{"command", "params"}` in, one condensed JSON object out (`{"success":...}`
or `{"error":"..."}`). Strictly one client: `HandleClient` runs inline on the accept thread (`:326`).

**Command execution.** The socket thread parses a line, posts the handler to the game thread with
`AsyncTask(ENamedThreads::GameThread, ...)` and blocks on an `FEvent` for 30 s (`:447-453`); Python
waits 60 s (`bridge.py:94`). On timeout the client gets an error but the game-thread task still runs
and its result is dropped, so the next command can overlap it. No `IsInGameThread()` asserts, no
worker-thread offload, no re-entrancy guard, no modal-dialog handling.

**Tools.** C++ side: `RegisterHandler(name, TFunction)` into a mutex-guarded `TMap` (`:308`), 95
handlers in one 8.9k-line file, all registered from `Start()`. Server side: each router module
exports `get_tools()` (JSON-Schema `Tool`) and `get_handlers()`; `main.py` merges them by hand.
The two name lists are independent and already drift (`get_widget_property` exists in C++ only).

**Images.** None over the wire. `screenshot` (`:3842`) takes a Slate screenshot of the level
viewport widget, PNG-encodes it on the game thread, writes `Saved/UnrealMCP/Screenshots/<name>.png`
and returns the path. `call_tool` returns only `TextContent` (`main.py:55-62`). `FBase64` is included
but unused. There is no viewport-camera control and no scene-capture component anywhere.

**PIE.** `start_pie`/`stop_pie` are fire-and-forget requests; `get_pie_status`, `press_key`
(timer-based auto-release holding a raw `APlayerController*`), `mouse_look` (yaw/pitch injection),
`mouse_click`, `get_game_state` (hardcoded 3000 uu radius), `call_component_function`.

**Config / tests / CI.** No `UDeveloperSettings`, no ini keys, paths hardcoded under
`ProjectSavedDir()`. No automation tests, no pytest, no CI. Docs are `README.md` and the `Tool`
description strings. Package targets UE 5.7 (`.uplugin`), README claims 5.5+; no version ifdefs.

## How the new pieces fit

```
MCP client ── stdio ── Python server ── TCP (JSON lines) ── plugin socket thread ── game thread
                         │                                        │
                         │ image content (base64 jpeg/png)        │ capture: trigger, readback, hand off
                         │ resources: manifest.json               ▼
                         └── recordings read from disk      worker thread: encode, disk I/O, JSON
                                                            Saved/MCPRecordings/<session>/...
```

- **Capture (Phase 1)** adds a `FMCPCapture` service in a new source file (not the 8.9k-line
  handler file): `USceneCaptureComponent2D` + render target for arbitrary cameras; the active
  viewport client for `"editor"`/`"pie"`. The game-thread handler only triggers the capture,
  enqueues a render-thread readback, and hands the pixel buffer to a worker that encodes and
  builds the response. Visible-actor projection uses the same view/projection matrices.
- **Wire format for images.** The JSON-lines protocol stays; image bytes travel as a base64 field
  (`"image": {"format", "w", "h", "b64"}`). The Python side maps that to MCP `ImageContent` next to
  a `TextContent` carrying the structured data. A 1024x576 JPEG at q80 is ~60-120 KB, well under
  any practical line size; the 30 s game-thread wait stays the bound.
- **Recording (Phase 2)** is a game-thread `FTSTicker` aligned to world time driving the same
  capture path at a low rate, plus an `FOutputDevice` registered for the session and a per-frame
  actor sampler. Everything lands under `Saved/MCPRecordings/<session>/` and is queried by later
  tools from disk, never from memory. Session lifetime hooks: PIE end delegate, module shutdown.
- **Settings.** One `UMCPCaptureSettings : UDeveloperSettings` (config=Editor) for resolution
  defaults, JPEG quality, storage path, retention, resolution cap; the first settings object in
  the plugin.
- **Threading contract.** `check(IsInGameThread())` at capture trigger/readback; `check(!IsInGameThread())`
  at encode/write. The socket thread keeps its current role; long captures never exceed the 30 s
  wait because the game-thread part is a trigger, not the encode.
- **Registration.** New handlers register through the existing `RegisterHandler` and a new Python
  router `routers/capture.py`; tool names follow the plan exactly (`capture_viewport`, `record_pie`, ...).

## Decisions taken before Phase 1 (transport hardening PR)

- **Single client stays.** Known limitation for v1: the accept loop serves one connection at a
  time, so a second MCP client (or a poller) waits until the first disconnects.
- **Structured errors.** Transport errors and every new tool answer
  `{"success": false, "error": {"code", "message", "hint"}}`. Codes so far: `invalid_json`,
  `unknown_command`, `timeout`, `busy`, `handler_failed`. The Python bridge flattens the object
  into `error` = `"code: message (hint)"` and keeps it under `error_detail`, so the 90+ legacy
  handlers that only read `resp["error"]` keep working.
- **Timeout no longer overlaps commands.** A command that outlives the wait (30 s, settable) is
  answered with `timeout`; if it had not started it is cancelled, if it is running it is marked
  stale and every further command is refused with `busy` until it finishes. Nothing is ever
  dispatched on top of a running command. The wait is not raised: that would only hide the stall.
- **Bytes on the wire.** The receive loop accumulates bytes and decodes a line once its
  newline arrives, so multi-byte UTF-8 and multi-MB lines (base64 images) are safe. Both sides
  cap a line at 64 MB: the plugin answers `line_too_long` and drops the client (draining first,
  so the error line is not lost to a TCP reset); the Python reader limit matches. A 5 MB round
  trip with a multi-byte tail is a regression test on both sides.
- **No blind resend.** If the connection drops after a write, the command may already have
  run on the game thread. The bridge reconnects for the next call and returns
  `connection_lost` with a hint to check state; only commands on an explicit idempotent
  allowlist (`ping`, `list_*`, `get_*`, `find_*`, `read_*`, `level_info`) are retried once.
- **`Python/src` is the source of truth**; `Content/Python/unreal_mcp` is regenerated by
  `Python/tools/sync_content.py` and a test fails on divergence. The vendored SDK under
  `Content/Python/Lib` is a packaging artifact.
- **Parity test.** `Python/tests/test_parity.py` enumerates C++ `RegisterHandler` names and
  Python `send_command` names and fails on any mismatch; every new tool goes through it.
- **Test harness.** UE automation tests live in `Source/UnrealMCP/Private/Tests/`
  (`UnrealMCP.Transport.*`, latent, socket work on a worker thread); Python tests under
  `Python/tests` (`pytest`, `-m live` for the ones that need an editor).

## Phase 1 threading decision (written before the first capture code)

Where each step of a capture runs, and why:

| step | thread | why |
|---|---|---|
| choose camera; for `"editor"`/`"pie"` draw the live viewport on demand (editor) and read it back with `FViewport::ReadPixels`; for explicit cameras spawn a transient `USceneCaptureComponent2D` and `CaptureScene()` | game | engine requirement: viewports, scene components and scene capture are game-thread APIs. The live viewport is read (not re-rendered through a scene capture) because it is the only path that carries every feature the user sees; a scene capture of the same camera measured 13x darker on a night scene |
| pixel readback (`FViewport::ReadPixels` / `FRenderTarget::ReadPixels`) | game thread, blocking on the render thread | the readback flushes rendering commands and copies the frame back; it is the one unavoidable stall (9-13 ms for a viewport frame, 55-160 ms for a scene capture; reported in the Phase 1 PR) and it keeps the pixel buffer's lifetime trivial |
| project actor bounds, occlusion line traces | game | world queries are game-thread only. Projection is O(actors) and cheap (about 1 us per actor); the line traces are bounded by `max_actors` (largest on-screen boxes first, the rest culled as `over_limit`), so the trace cost follows the request while the projection pass remains O(actors) |
| resize to the requested size, JPEG/PNG encode, base64, JSON assembly | **socket thread**, after the game-thread part has returned | the socket thread already exists per command and is not the game thread; no thread pool, no extra copies |

Mechanism: a handler running on the game thread may call
`FMCPTcpServer::QueuePostProcess(TFunction<void(TSharedPtr<FJsonObject>&)>)`. The server stores
the lambda on the pending command; once the game-thread part has finished (and only if it did
not time out) the socket thread runs it on the result before sending. The pixel buffer is moved
into the lambda, so the game thread never encodes and the game-thread part of `capture_viewport`
is exactly: trigger, read back, hand off. `check(IsInGameThread())` guards the capture path and
`check(!IsInGameThread())` guards the post-process path.

Not chosen: a separate encode thread pool (nothing to gain while the transport is single-client
and one command is in flight at a time). The synchronous readback is accepted for one-shot
captures only; see the Phase 2 decisions below.

## Phase 2 decisions (recorded before the recording code)

Two Phase 1 shortcuts are right for a one-shot capture and wrong for a recorder. They are the
plan, not open questions:

1. **The readback never blocks the game thread.** `FViewport::ReadPixels` and
   `FRenderTarget::ReadPixels` flush the render thread and copy synchronously: 9-13 ms per
   viewport frame (55-160 ms for a scene capture). At 10 fps that is 100+ ms of game-thread
   stall per second, hitching the very PIE session being recorded. The recorder uses
   `FRHIGPUTextureReadback` with a one-frame delay: on frame N a render command enqueues a copy
   of the frame into the readback's staging texture; on frame N+1 (or whenever `IsReady()`
   first reports true) a render command locks the staging texture, copies the pixels out and
   unlocks (`Lock`/`Unlock` are render-thread calls), then posts the buffer to the session's
   own encode worker: one `FRunnableThread` per recording session that encodes and writes
   under `Saved/MCPRecordings/<session>/`. `QueuePostProcess` is not usable here: it belongs
   to the one command in flight, and a recording outlives its command. The price is one frame
   of latency between the sampled game state and the image; the timeline stores the frame's
   own world time, so nothing is misattributed. Lowering the recording fps cap is not an
   acceptable substitute: it hides the stall instead of removing it.
2. **Frames come from the live PIE viewport; one pooled set of capture objects per session.**
   The Phase 1 finding stands for recording: the PIE viewport's render target is the only
   source that carries what the player sees, so `record_pie` reads that texture through the
   session's `FRHIGPUTextureReadback` (double-buffered so frame N+1 can be enqueued while
   frame N is copied out). Only a recording of a camera other than the PIE viewport (not in
   the Phase 2 contract, but the path exists for explicit-camera captures) renders through a
   scene capture; `capture_viewport` allocates a `USceneCaptureComponent2D` and a render
   target for every explicit-camera call and marks both as garbage after the readback, which
   is fine once and not ten times a second. A recording session therefore owns its readback
   objects and, when it needs one, one scene-capture component + render target, all created
   by `record_pie` and released by `stop_recording`, the PIE-end delegate or module shutdown,
   whichever comes first.

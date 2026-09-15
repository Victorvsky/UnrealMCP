# Visual capture tools

Tool reference for the visual-capture family. Every tool returns structured errors as
`{"error": {"code", "message", "hint"}}`; the Python server flattens them to text for the client.

## `capture_viewport`

One frame plus the engine state that explains it. The frame is returned as MCP image content;
the state follows as a JSON text block.

**Input**

| field | type | default | notes |
|---|---|---|---|
| `camera` | `"editor"` \| `"pie"` \| `{location:{x,y,z}, rotation:{pitch,yaw,roll}, fov?}` | `"pie"` if a game is running, else `"editor"` | `fov` is horizontal degrees (default 90, clamped 5-170) |
| `world` | `"editor"` \| `"pie"` | PIE if running | explicit cameras only: which world to render |
| `resolution` | `{w, h}` | `1024x576` (Project Settings > Plugins > MCP Capture) | longest edge capped by `MaxLongEdge` (2048) |
| `format` | `"jpeg"` \| `"png"` | `"jpeg"` | |
| `quality` | 1-100 | 80 | JPEG only |
| `max_actors` | int, 0-2000 | 200 | bounds the list and the occlusion traces: the `max_actors` largest on-screen boxes are traced, the rest are `over_limit`; values above 2000 are clamped; 0 lists nothing (with `debug`, every candidate is `over_limit`) |
| `debug` | bool | false | also return `culled`: `[{name, reason}]` with `no_rendered_mesh`, `behind_camera`, `off_screen`, `over_limit`, `occluded_by:<actor>` |

**Output** (the JSON text block; the image travels separately)

```json
{
  "timestamp": 12.5, "timestamp_kind": "world_time",          // "unix_time" for editor captures
  "camera": {"location": {..}, "rotation": {..}, "fov": 90, "world": "pie"},
  "actors": [
    {"name": "BP_LampPost15", "class": "BP_LampPost_C",
     "screen_bbox": [584, 0, 705, 531], "world_location": {..}, "distance": 895.0}
  ],
  "image": {"format": "jpeg", "width": 1024, "height": 576, "mime_type": "image/jpeg", "bytes": 63120},
  "timings": {"game_thread_capture_ms": 56.4, "game_thread_actors_ms": 0.9, "encode_ms": 1.2}
}
```

`actors` lists actors whose rendered mesh bounds project into the image and are not hidden
behind something else (one line trace to the bounds centre; a partly hidden actor still
counts as visible). The traces are bounded by `max_actors`: candidates are ordered by
on-screen box area and only the first `max_actors` are traced, so the trace count follows
the request, not the level. The projection pass still visits every actor in the level (about
1 us each; see Measured below). The list itself is nearest first. Volumes, info actors,
brushes and the per-map foliage actor are skipped.
`screen_bbox` is `[x0, y0, x1, y1]` in pixels, clamped to the image. Large actors such as the
landscape or a skybox legitimately cover the whole frame.

**Errors**

| code | when |
|---|---|
| `pie_not_running` | `camera: "pie"` with no Play-In-Editor session |
| `no_player` | PIE is running but no player controller exists yet |
| `no_editor_viewport` | `camera: "editor"` with no level viewport open |
| `invalid_camera` | malformed explicit camera or unknown mode |
| `invalid_resolution` / `resolution_too_large` | below 16 px, or longest edge above the cap |
| `invalid_format` | not jpeg/png |
| `capture_failed` / `encode_failed` | renderer readback or image encoding failed |

**How it renders.** `"editor"` and `"pie"` read the live viewport's last frame back (the editor
viewport is drawn on demand first), so the image carries everything the user sees: Lumen, post
process, converged exposure. Measured against a viewport screenshot at the same camera: mean
luminance 6.2 vs 6.1 (editor), 11.0 vs 12.4 (PIE, at a different JPEG size). The viewport keeps
its own aspect ratio: the requested size is fitted around it and the actual `width`/`height` are
reported (a 2.9:1 editor viewport asked for 1024x576 returns 1024x347). Explicit cameras have no
viewport and render through a transient `USceneCaptureComponent2D` (final tonemapped colour, view
state kept, instant adaptation, two renders); that path lacks what the scene capture cannot do
(on this project it renders night scenes markedly darker than the viewport), which `camera.source`
reports as `"scene_capture"` versus `"viewport"`. Pixels are read back on the game thread;
resize, JPEG/PNG encoding and base64 run on the socket thread after the handler has returned
(see ARCHITECTURE.md).

**Measured** (UE 5.7, 391-actor level, loopback client): game-thread part 9-13 ms for a
viewport readback (editor: plus the on-demand draw, 190 ms on the first call), 55-160 ms for a
scene capture (two renders plus readback); visible-actor pass 0.7-1.3 ms on the 391-actor main
level (375 actors considered, at most 200 traces at the default cap) and 1.9-2.5 ms on a
1,721-actor level, where the per-actor projection pass dominates (about 1.1 us per actor)
and the traces stay capped; resize plus encode about 10 ms. End to end the call waits for the next editor frame, so the round trip is bounded
by the editor's frame rate: 40-80 ms at interactive rates, 330 ms when the editor is throttled
to 3 FPS in the background.

## `record_pie`

Starts recording the Play-In-Editor session and returns at once with a session id. The session
ends on its own after `duration_s` seconds of world time, when PIE ends, or on `stop_recording`.
One session records at a time (`recording_in_progress` otherwise).

**Input**

| field | type | default | notes |
|---|---|---|---|
| `duration_s` | 0-120 | 30 | world time, not wall time |
| `fps` | 0-10 | 2 | frames per second of world time |
| `resolution` | `{w, h}` | Project Settings default | bounding box; the viewport aspect ratio is kept, longest edge capped by `MaxLongEdge` |
| `actor_filter` | string[] | pawns + actors tagged `MCPTrack` | class names (any ancestor) or actor tags to track for actor events |
| `start_pie` | bool | true | start PIE when none runs; a session the tool starts runs with a fixed 1/30 s time step and a fixed random seed so two runs are comparable |
| `quality` | 1-100 | Project Settings JPEG quality | |

**Output**

```json
{"session_id": "20260915-190612-3f9a1c", "state": "recording",      // "starting" while PIE boots
 "manifest_uri": "file:///F:/Eruin/Saved/MCPRecordings/20260915-190612-3f9a1c/manifest.json",
 "started_pie": false, "duration_s": 30, "fps": 2, "dir": "F:/Eruin/Saved/MCPRecordings/20260915-190612-3f9a1c"}
```

**Storage** (`Saved/<StoragePath>/<session>/`, StoragePath defaults to `MCPRecordings`,
sessions older than `RetentionDays` are pruned when the plugin starts):

| file | content |
|---|---|
| `manifest.json` | state, counts, timings, level, resolution; rewritten after every frame, so it is always valid |
| `frames/NNNNNN.jpg` | one JPEG per captured frame |
| `frames.jsonl` | one record per frame: `{index, t, world_time, file, width, height, bytes, actors:[...]}` |
| `timeline.jsonl` | `{t, kind: "log"|"actor"|"stats", ...}` records in the order they happened |

`t` is seconds since the recording started; `world_time` is the PIE world clock. Every record
carries the world time of the frame it belongs to: the image copied on tick N is frame N-1's
render, and the actor sample taken at the start of tick N is frame N-1's world state, so the
two agree. The manifest is also exposed as an MCP resource (`file:///...manifest.json`).

**Errors:** `recording_in_progress` (with `session_id`), `pie_not_running` (only with
`start_pie: false`), `invalid_duration`, `invalid_fps`, `invalid_resolution`,
`resolution_too_large`, `storage_failed`.

## `get_recording_status`

`{session_id?}` -> the manifest, live from memory for the running session (`active: true`) or
from disk otherwise. Fields: `state` (`starting` | `recording` | `finished`), `end_reason`
(`duration` | `stopped` | `pie_ended` | `shutdown` | `failed`), `frames`, `dropped_frames`,
`warnings`, `errors`, `log_lines`, `actor_events`, `recorded_s`, `source_format` (the
viewport's pixel format, `sync:...` when the synchronous fallback had to be used),
`timings.game_thread_ms_avg` / `game_thread_ms_max` (the per-captured-frame cost of the
game-thread part: enqueue + actor sample), `timings.worker_encode_ms_avg`, `notes` (dropped
frame reasons and other one-line diagnostics). Without `session_id`: the running session, else
the last one of this editor run. Errors: `session_not_found`, `invalid_session`.

## `stop_recording`

`{session_id?}` -> the final manifest of the running session. `not_recording` for a session
that already finished (its manifest is on disk; use `get_recording_status`).

## `get_recording_frames`

`{session_id, start_s?, end_s?, fps?, max_frames?}`. Grid points `start_s + k / fps` up to
`end_s` (default: everything recorded so far), each matched to the nearest unused recorded
frame within half a grid interval; the others come back `available: false`. `fps` defaults to
the recorded rate, so the default call returns every recorded frame in the window. `max_frames`
(default 8, max 32) caps the grid and `truncated` reports when the window had more. Works on a
session that is still recording: frames land on disk as the worker finishes them.

```json
{"session_id": "...", "recorded_fps": 2, "requested_fps": 4, "start_s": 0, "end_s": 1, "available": 3, "truncated": false,
 "frames": [
   {"t": 0.0,  "available": true, "index": 0, "recorded_t": 0.0, "world_time": 12.5,
    "image": {"format": "jpeg", "mime_type": "image/jpeg", "width": 1024, "height": 347, "bytes": 41210, "data": "..."},
    "actors": [{"name": "BP_Alder", "class": "BP_Alder_C", "screen_bbox": [..], "world_location": {..}, "distance": 512.0}]},
   {"t": 0.25, "available": false, "reason": "not available: no recorded frame near this time"}
 ]}
```

The Python server returns a header text block (the grid, with which points are available), then
for every available frame an image followed by its state as JSON text.

## `get_recording_timeline`

`{session_id, start_s?, end_s?}` ->

```json
{"log":          [{"t": 3.2, "verbosity": "Warning", "category": "LogTemp", "message": "..."}],
 "actor_events": [{"t": 3.5, "actor": "BP_Alder", "event": "moved", "detail": {"from": {..}, "to": {..}, "distance": 42.0, "yaw": 90}}],
 "stats":        [{"t": 3.5, "fps": 58.1, "frame_ms": 17.2, "draw_calls": 1820}],
 "truncated":    {"log": false, "actor_events": false, "stats": false}}
```

Log lines are captured at verbosity Log and above (Verbose and VeryVerbose are not), minus the
plugin's own transport chatter; the first 20,000 lines of a session are kept. Actor events:
`tracked` (present at the first sample), `spawned`, `destroyed`, `moved` (more than 10 units or
2 degrees since the last sample), `state_changed` (a Blueprint-visible bool or number on the
actor changed; found through reflection, nothing game-specific; at most 32 properties per
actor, 8 changes per sample). Stats are sampled at the recording rate. Caps per query: 2,000
log lines, 2,000 actor events, 4,000 stats records, each with its own `truncated` flag.

**How it records.** A game-thread ticker enqueues a GPU copy of the PIE viewport's render
target into an `FRHIGPUTextureReadback` (two in flight) and samples actors; on later frames a
render command copies out the readbacks that are ready (never waiting); a per-session worker
thread converts, resizes, encodes and writes. The game thread never waits for the GPU or the
disk, so the per-frame cost is the enqueue plus the actor sample. If the viewport has no
separate render target (it draws straight into the window), frames fall back to a synchronous
readback and the manifest says so in `notes` and `source_format`.

**Measured** (UE 5.7, Eruin's main level, PIE in the editor's 2.96:1 level viewport, editor in the foreground):

| | |
|---|---|
| game-thread cost per captured frame (enqueue the GPU copy, project visible actors, sample tracked actors, stats record) | 0.69 ms avg / 0.88 ms max at 2 fps with a 320x180 request; 0.79 ms avg / 0.89 ms max at 2 fps with the default 1024x576 request |
| dropped frames | 0 of 8, 0 of 12 |
| worker thread per frame (10-bit to BGRA, resize, JPEG, write) | 11.2 ms for 320x108 output, 13.5 ms for 1024x347 output; not on the game thread |
| viewport render target format | `A2B10G10R10` (the editor's default back buffer), converted on the worker |
| acceptance (< 3 ms per frame on the game thread at 2 fps, 1024x576) | met at 0.79 ms |

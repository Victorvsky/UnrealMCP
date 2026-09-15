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

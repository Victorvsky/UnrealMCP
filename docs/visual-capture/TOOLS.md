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
| `max_actors` | int | 200 | nearest first |
| `debug` | bool | false | also return `culled`: `[{name, reason}]` with `no_rendered_mesh`, `behind_camera`, `off_screen`, `occluded_by:<actor>` |

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
counts as visible). Volumes, info actors, brushes and the per-map foliage actor are skipped.
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

**How it renders.** A transient `USceneCaptureComponent2D` renders the requested camera into an
8-bit target (final tonemapped colour). Auto-exposed scenes have no adaptation history in a
one-shot capture, so the capture keeps its view state, sets instant adaptation and renders
twice; the result matches the editor viewport's exposure (measured: same mean luminance on a
night scene). Pixels are read back on the game thread; JPEG/PNG encoding and base64 run on the
socket thread after the handler has returned (see ARCHITECTURE.md).

**Measured** (UE 5.7, 391-actor level, loopback client): default 1024x576 JPEG round trip
70-200 ms; of that the game-thread part is 55-160 ms (two scene renders plus readback), the
visible-actor pass under 1 ms, encoding about 1 ms. The first capture after editor start is the
slow one (render-target allocation).

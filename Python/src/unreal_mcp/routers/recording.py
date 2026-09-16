# Copyright (c) 2026 victorvksy. All rights reserved.

"""PIE recording tools: record a play session to disk, then query frames and the timeline."""

import json
import os
from pathlib import Path

from mcp.types import Tool, TextContent, ImageContent

from unreal_mcp.bridge import bridge

SESSION_ID = {"type": "string", "description": "Session id from record_pie. Omitted: the running session, else the last one of this editor run."}


def get_tools() -> list[Tool]:
    return [
        Tool(
            name="record_pie",
            description=(
                "Start recording the Play-In-Editor session: frames at a low rate plus a timeline "
                "(log lines, tracked-actor events, frame stats), written under Saved/MCPRecordings/<session>. "
                "Returns immediately with a session_id; the recording ends on its own after duration_s, when PIE "
                "ends, or on stop_recording. Poll get_recording_status; query get_recording_frames / "
                "get_recording_timeline at any time, also while it is still recording."
            ),
            inputSchema={
                "type": "object",
                "properties": {
                    "duration_s": {"type": "number", "minimum": 0, "maximum": 120, "description": "Seconds of world time to record (default 30, max 120)."},
                    "fps": {"type": "number", "minimum": 0, "maximum": 10, "description": "Frames per second of world time (default 2, max 10)."},
                    "resolution": {
                        "type": "object",
                        "properties": {"w": {"type": "integer"}, "h": {"type": "integer"}},
                        "description": "Frame size bounding box (default from Project Settings > MCP Capture; the viewport aspect ratio is kept).",
                    },
                    "actor_filter": {
                        "type": "array",
                        "items": {"type": "string"},
                        "description": "Class names or actor tags to track for actor events. Default: all pawns plus actors tagged \"MCPTrack\".",
                    },
                    "start_pie": {"type": "boolean", "description": "Start PIE if it is not running (default true). A session the tool starts runs with a fixed 1/30 s time step and a fixed random seed so runs are comparable."},
                    "quality": {"type": "integer", "minimum": 1, "maximum": 100, "description": "JPEG quality override."},
                },
            },
        ),
        Tool(
            name="get_recording_status",
            description="State of a recording session: recording/finished, frames written, warnings and errors seen, game-thread cost per frame, storage location.",
            inputSchema={"type": "object", "properties": {"session_id": SESSION_ID}},
        ),
        Tool(
            name="stop_recording",
            description="Stop the running recording session early and return its final manifest.",
            inputSchema={"type": "object", "properties": {"session_id": SESSION_ID}},
        ),
        Tool(
            name="get_recording_frames",
            description=(
                "Frames of a recording between start_s and end_s (seconds since the recording started) on a time "
                "grid of fps (default: the recorded rate), each as an image with the visible actors at that time. "
                "Grid points with no recorded frame nearby are reported as not available. Works on a session that is still recording."
            ),
            inputSchema={
                "type": "object",
                "properties": {
                    "session_id": SESSION_ID,
                    "start_s": {"type": "number", "minimum": 0, "description": "Start of the window (default 0)."},
                    "end_s": {"type": "number", "minimum": 0, "description": "End of the window (default: everything recorded so far)."},
                    "fps": {"type": "number", "description": "Grid rate (default: the recorded fps; higher rates return gaps as not available)."},
                    "max_frames": {"type": "integer", "minimum": 1, "maximum": 32, "description": "Cap on returned grid points (default 8); `truncated` says when the window had more."},
                },
            },
        ),
        Tool(
            name="get_recording_timeline",
            description=(
                "What happened during a recording: log lines (Log and above, with verbosity and category), tracked-actor "
                "events (tracked/spawned/destroyed/moved/state_changed) and per-frame stats (fps, frame_ms, draw_calls), "
                "optionally limited to [start_s, end_s]. Works on a session that is still recording."
            ),
            inputSchema={
                "type": "object",
                "properties": {
                    "session_id": SESSION_ID,
                    "start_s": {"type": "number", "minimum": 0},
                    "end_s": {"type": "number", "minimum": 0},
                },
            },
        ),
    ]


def _params(args: dict, keys: tuple[str, ...]) -> dict:
    return {k: args[k] for k in keys if args.get(k) is not None}


def _text(resp: dict) -> str:
    if "error" in resp:
        return f"Error: {resp['error']}"
    return json.dumps({k: v for k, v in resp.items() if k != "success"}, indent=1)


async def handle_record_pie(args: dict) -> str:
    resp = await bridge.send_command("record_pie", _params(args, ("duration_s", "fps", "resolution", "actor_filter", "start_pie", "quality")))
    return _text(resp)


async def handle_get_recording_status(args: dict) -> str:
    resp = await bridge.send_command("get_recording_status", _params(args, ("session_id",)))
    return _text(resp)


async def handle_stop_recording(args: dict) -> str:
    resp = await bridge.send_command("stop_recording", _params(args, ("session_id",)))
    return _text(resp)


def build_frames_content(resp: dict) -> list[TextContent | ImageContent]:
    """Header text, then for every available frame an image followed by its state as JSON text.
    Grid points without a frame are listed in the header only."""
    if "error" in resp:
        return [TextContent(type="text", text=f"Error: {resp['error']}")]
    frames = resp.get("frames") or []
    header = {k: v for k, v in resp.items() if k not in ("frames", "success")}
    header["grid"] = [
        {"t": f.get("t"), "available": f.get("available", False), **({"index": f.get("index")} if f.get("available") else {"reason": f.get("reason")})}
        for f in frames
    ]
    content: list[TextContent | ImageContent] = [TextContent(type="text", text=json.dumps(header, indent=1))]
    for f in frames:
        if not f.get("available"):
            continue
        image = f.get("image") or {}
        data = image.get("data")
        if not data:
            continue
        meta = {k: v for k, v in f.items() if k != "image"}
        meta["image"] = {k: v for k, v in image.items() if k != "data"}
        content.append(ImageContent(type="image", data=data, mimeType=image.get("mime_type", "image/jpeg")))
        content.append(TextContent(type="text", text=json.dumps(meta, indent=1)))
    return content


async def handle_get_recording_frames(args: dict) -> list[TextContent | ImageContent]:
    resp = await bridge.send_command("get_recording_frames", _params(args, ("session_id", "start_s", "end_s", "fps", "max_frames")))
    return build_frames_content(resp)


async def handle_get_recording_timeline(args: dict) -> str:
    resp = await bridge.send_command("get_recording_timeline", _params(args, ("session_id", "start_s", "end_s")))
    return _text(resp)


def get_handlers() -> dict:
    return {
        "record_pie": handle_record_pie,
        "get_recording_status": handle_get_recording_status,
        "stop_recording": handle_stop_recording,
        "get_recording_frames": handle_get_recording_frames,
        "get_recording_timeline": handle_get_recording_timeline,
    }


# --- manifests as MCP resources -------------------------------------------------------------

def storage_root() -> Path | None:
    """Saved/MCPRecordings of the project this server talks to, if it can be located."""
    candidates = []
    project = os.environ.get("UE_PROJECT_PATH")
    if project:
        candidates.append(Path(project))
    here = Path(__file__).resolve()
    for p in (here.parents[5] if len(here.parents) > 5 else None, here.parents[6] if len(here.parents) > 6 else None):
        if p is not None:
            candidates.append(p)
    for root in candidates:
        saved = root / "Saved" / "MCPRecordings"
        if saved.is_dir():
            return saved
    return None


def list_manifests() -> list[dict]:
    root = storage_root()
    if root is None:
        return []
    out = []
    for manifest in sorted(root.glob("*/manifest.json"), reverse=True):
        out.append({"uri": manifest.resolve().as_uri(), "name": f"recording {manifest.parent.name}", "mimeType": "application/json"})
    return out


def read_manifest(uri: str) -> str | None:
    root = storage_root()
    if root is None or not uri.startswith("file:"):
        return None
    from urllib.parse import unquote, urlparse

    path = Path(unquote(urlparse(uri).path.lstrip("/"))) if os.name == "nt" else Path(unquote(urlparse(uri).path))
    try:
        path.resolve().relative_to(root.resolve())
    except ValueError:
        return None
    if path.name != "manifest.json" or not path.is_file():
        return None
    return path.read_text(encoding="utf-8")

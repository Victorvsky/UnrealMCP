# Copyright (c) 2026 victorvksy. All rights reserved.

"""Visual capture tools: frames returned together with the engine state that explains them."""

import json

from mcp.types import Tool, TextContent, ImageContent

from unreal_mcp.bridge import bridge


def get_tools() -> list[Tool]:
    return [
        Tool(
            name="capture_viewport",
            description=(
                "Capture one frame from the editor viewport, the PIE player camera, or an explicit "
                "camera, and return it as an image together with the camera pose and the list of "
                "visible actors (name, class, screen bounding box, world location, distance). "
                "Default camera: the PIE camera if a game is running, otherwise the editor viewport."
            ),
            inputSchema={
                "type": "object",
                "properties": {
                    "camera": {
                        "description": "\"editor\", \"pie\", or {location:{x,y,z}, rotation:{pitch,yaw,roll}, fov?}.",
                        "oneOf": [
                            {"type": "string", "enum": ["editor", "pie"]},
                            {
                                "type": "object",
                                "properties": {
                                    "location": {"type": "object", "properties": {"x": {"type": "number"}, "y": {"type": "number"}, "z": {"type": "number"}}, "required": ["x", "y", "z"]},
                                    "rotation": {"type": "object", "properties": {"pitch": {"type": "number"}, "yaw": {"type": "number"}, "roll": {"type": "number"}}, "required": ["pitch", "yaw", "roll"]},
                                    "fov": {"type": "number", "description": "Horizontal field of view in degrees (default 90)."},
                                },
                                "required": ["location", "rotation"],
                            },
                        ],
                    },
                    "world": {
                        "type": "string",
                        "enum": ["editor", "pie"],
                        "description": "For an explicit camera only: which world to render (default: PIE if running, else editor).",
                    },
                    "resolution": {
                        "type": "object",
                        "properties": {"w": {"type": "integer"}, "h": {"type": "integer"}},
                        "description": "Image size (default 1024x576; longest edge capped, see Project Settings > MCP Capture).",
                    },
                    "format": {"type": "string", "enum": ["jpeg", "png"], "description": "Default jpeg (quality 80)."},
                    "quality": {"type": "integer", "minimum": 1, "maximum": 100, "description": "JPEG quality override."},
                    "max_actors": {"type": "integer", "minimum": 0, "maximum": 2000, "description": "Cap on the visible-actor list (nearest first) and on the occlusion traces: the max_actors largest on-screen boxes are traced, the rest are culled as over_limit. Hard ceiling 2000; 0 lists nothing (with debug, every candidate is reported as over_limit)."},
                    "debug": {"type": "boolean", "description": "Also return `culled`: every skipped actor with the reason (no_rendered_mesh, behind_camera, off_screen, over_limit, occluded_by:<actor>)."},
                },
            },
        ),
    ]


def build_capture_content(resp: dict) -> list[TextContent | ImageContent]:
    """Turn a plugin capture response into MCP content: the image, then the state as JSON text."""
    if "error" in resp:
        return [TextContent(type="text", text=f"Error: {resp['error']}")]
    image = resp.get("image") or {}
    data = image.get("data")
    if not data:
        return [TextContent(type="text", text="Error: capture returned no image data")]
    meta = {k: v for k, v in resp.items() if k not in ("image", "success")}
    meta["image"] = {k: v for k, v in image.items() if k != "data"}
    return [
        ImageContent(type="image", data=data, mimeType=image.get("mime_type", "image/jpeg")),
        TextContent(type="text", text=json.dumps(meta, indent=1)),
    ]


async def handle_capture_viewport(args: dict) -> list[TextContent | ImageContent]:
    params = {}
    for key in ("camera", "world", "resolution", "format", "quality", "max_actors", "debug"):
        if args.get(key) is not None:
            params[key] = args[key]
    resp = await bridge.send_command("capture_viewport", params)
    return build_capture_content(resp)


def get_handlers() -> dict:
    return {"capture_viewport": handle_capture_viewport}

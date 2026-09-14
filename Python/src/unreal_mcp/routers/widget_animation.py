# Copyright (c) 2026 victorvksy. All rights reserved.

"""Widget animation tools — list, create, read, and edit UMG widget animations."""

from mcp.types import Tool

from unreal_mcp.bridge import bridge


def get_tools() -> list[Tool]:
    return [
        Tool(
            name="list_widget_animations",
            description=(
                "List all animations on a Widget Blueprint. "
                "Returns name, duration, bound widgets, and track count for each."
            ),
            inputSchema={
                "type": "object",
                "properties": {
                    "widget_blueprint": {
                        "type": "string",
                        "description": "Widget Blueprint name or path (e.g. 'WBP_HUD').",
                    },
                },
                "required": ["widget_blueprint"],
            },
        ),
        Tool(
            name="read_widget_animation",
            description=(
                "Read detailed info about a widget animation: bindings, tracks, "
                "and playback range. Use this to understand animation structure before editing."
            ),
            inputSchema={
                "type": "object",
                "properties": {
                    "widget_blueprint": {
                        "type": "string",
                        "description": "Widget Blueprint name or path.",
                    },
                    "animation_name": {
                        "type": "string",
                        "description": "Display name of the animation.",
                    },
                },
                "required": ["widget_blueprint", "animation_name"],
            },
        ),
        Tool(
            name="create_widget_animation",
            description=(
                "Create a new animation on a Widget Blueprint. "
                "After creation, use add_widget_animation_track to add tracks for specific widgets."
            ),
            inputSchema={
                "type": "object",
                "properties": {
                    "widget_blueprint": {
                        "type": "string",
                        "description": "Widget Blueprint name or path.",
                    },
                    "animation_name": {
                        "type": "string",
                        "description": "Display name for the new animation.",
                    },
                    "duration": {
                        "type": "number",
                        "description": "Duration in seconds (default 1.0).",
                    },
                },
                "required": ["widget_blueprint", "animation_name"],
            },
        ),
        Tool(
            name="add_widget_animation_track",
            description=(
                "Add an animation track for a widget within a widget animation. "
                "Binds the specified widget and adds a track of the given type. "
                "Returns the binding_guid needed for adding keyframes."
            ),
            inputSchema={
                "type": "object",
                "properties": {
                    "widget_blueprint": {
                        "type": "string",
                        "description": "Widget Blueprint name or path.",
                    },
                    "animation_name": {
                        "type": "string",
                        "description": "Animation name.",
                    },
                    "widget_name": {
                        "type": "string",
                        "description": "Name of the widget in the tree to animate.",
                    },
                    "track_type": {
                        "type": "string",
                        "description": "Track type: Float, Bool, Visibility, Transform.",
                        "enum": ["Float", "Bool", "Visibility", "Transform"],
                    },
                    "name": {
                        "type": "string",
                        "description": "Optional display name for the track.",
                    },
                },
                "required": ["widget_blueprint", "animation_name", "widget_name", "track_type"],
            },
        ),
        Tool(
            name="add_widget_animation_keyframe",
            description=(
                "Add a keyframe to a widget animation track. "
                "Requires the binding_guid from add_widget_animation_track."
            ),
            inputSchema={
                "type": "object",
                "properties": {
                    "widget_blueprint": {
                        "type": "string",
                        "description": "Widget Blueprint name or path.",
                    },
                    "animation_name": {
                        "type": "string",
                        "description": "Animation name.",
                    },
                    "binding_guid": {
                        "type": "string",
                        "description": "GUID of the widget binding (from add_widget_animation_track).",
                    },
                    "time": {
                        "type": "number",
                        "description": "Keyframe time in seconds.",
                    },
                    "value": {
                        "type": "number",
                        "description": "Keyframe value (float).",
                    },
                    "track_index": {
                        "type": "integer",
                        "description": "Index of the track on this binding (default 0).",
                    },
                    "channel_index": {
                        "type": "integer",
                        "description": "Channel index within the track (default 0).",
                    },
                    "interpolation": {
                        "type": "string",
                        "description": "Interpolation mode: Cubic, Linear, or Constant.",
                        "enum": ["Cubic", "Linear", "Constant"],
                    },
                },
                "required": ["widget_blueprint", "animation_name", "binding_guid", "time", "value"],
            },
        ),
    ]


def get_handlers() -> dict:
    return {
        "list_widget_animations": handle_list_widget_animations,
        "read_widget_animation": handle_read_widget_animation,
        "create_widget_animation": handle_create_widget_animation,
        "add_widget_animation_track": handle_add_widget_animation_track,
        "add_widget_animation_keyframe": handle_add_widget_animation_keyframe,
    }


async def handle_list_widget_animations(args: dict) -> str:
    resp = await bridge.send_command("list_widget_animations", {
        "widget_blueprint": args["widget_blueprint"],
    })
    if "error" in resp:
        return f"Error: {resp['error']}"

    anims = resp.get("animations", [])
    if not anims:
        return f"No animations on {resp.get('widget_blueprint', '')}."

    lines = [f"Widget Blueprint: {resp.get('widget_blueprint', '')}", f"Animations ({resp['count']}):\n"]
    for a in anims:
        dur = a.get("duration", 0)
        tracks = a.get("track_count", "?")
        widgets = ", ".join(a.get("bound_widgets", []))
        lines.append(f"  - {a['name']} ({dur:.2f}s, {tracks} tracks)")
        if widgets:
            lines.append(f"    Widgets: {widgets}")
    return "\n".join(lines)


async def handle_read_widget_animation(args: dict) -> str:
    resp = await bridge.send_command("read_widget_animation", {
        "widget_blueprint": args["widget_blueprint"],
        "animation_name": args["animation_name"],
    })
    if "error" in resp:
        return f"Error: {resp['error']}"

    lines = [
        f"Animation: {resp.get('name', '')}",
        f"Time: {resp.get('start_time', 0):.2f}s — {resp.get('end_time', 0):.2f}s",
        f"Display Rate: {resp.get('display_rate', '?')}",
    ]

    bindings = resp.get("bindings", [])
    if bindings:
        lines.append(f"\nBindings ({len(bindings)}):")
        for b in bindings:
            lines.append(f"  - {b['name']} (GUID: {b.get('guid', '?')})")
            for t in b.get("tracks", []):
                lines.append(f"      Track: {t['name']} ({t['type']}, {t.get('section_count', 0)} sections)")

    master = resp.get("master_tracks", [])
    if master:
        lines.append(f"\nMaster Tracks ({len(master)}):")
        for t in master:
            lines.append(f"  - {t['name']} ({t['type']})")

    return "\n".join(lines)


async def handle_create_widget_animation(args: dict) -> str:
    params = {
        "widget_blueprint": args["widget_blueprint"],
        "animation_name": args["animation_name"],
    }
    if "duration" in args:
        params["duration"] = args["duration"]

    resp = await bridge.send_command("create_widget_animation", params)
    if "error" in resp:
        return f"Error: {resp['error']}"

    return (
        f"Created animation '{resp.get('animation_name', '')}' "
        f"({resp.get('duration', 1.0):.1f}s) on {resp.get('widget_blueprint', '')}"
    )


async def handle_add_widget_animation_track(args: dict) -> str:
    params = {
        "widget_blueprint": args["widget_blueprint"],
        "animation_name": args["animation_name"],
        "widget_name": args["widget_name"],
        "track_type": args["track_type"],
    }
    if "name" in args:
        params["name"] = args["name"]

    resp = await bridge.send_command("add_widget_animation_track", params)
    if "error" in resp:
        return f"Error: {resp['error']}"

    return (
        f"Added {resp.get('track_type', '')} track for widget '{resp.get('widget', '')}' "
        f"in animation '{resp.get('animation', '')}'\n"
        f"Binding GUID: {resp.get('binding_guid', '')} (use this for keyframes)"
    )


async def handle_add_widget_animation_keyframe(args: dict) -> str:
    params = {
        "widget_blueprint": args["widget_blueprint"],
        "animation_name": args["animation_name"],
        "binding_guid": args["binding_guid"],
        "time": args["time"],
        "value": args["value"],
    }
    for opt in ("track_index", "channel_index", "interpolation"):
        if opt in args:
            params[opt] = args[opt]

    resp = await bridge.send_command("add_widget_animation_keyframe", params)
    if "error" in resp:
        return f"Error: {resp['error']}"

    return (
        f"Added keyframe at {resp.get('time', 0):.2f}s = {resp.get('value', 0)} "
        f"in animation '{resp.get('animation', '')}'"
    )

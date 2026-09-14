# Copyright (c) 2026 victorvksy. All rights reserved.

"""Playtest tools — PIE control, input simulation, and game state queries."""

from mcp.types import Tool

from unreal_mcp.bridge import bridge


def get_tools() -> list[Tool]:
    return [
        Tool(
            name="start_pie",
            description=(
                "Start a Play In Editor (PIE) session. "
                "This is asynchronous — use get_pie_status to confirm it started. "
                "Must not already be in a PIE session."
            ),
            inputSchema={
                "type": "object",
                "properties": {},
            },
        ),
        Tool(
            name="stop_pie",
            description=(
                "Stop the current Play In Editor (PIE) session and return to the editor."
            ),
            inputSchema={
                "type": "object",
                "properties": {},
            },
        ),
        Tool(
            name="get_pie_status",
            description=(
                "Check if a PIE session is currently running. "
                "Returns play time and player position if active."
            ),
            inputSchema={
                "type": "object",
                "properties": {},
            },
        ),
        Tool(
            name="press_key",
            description=(
                "Simulate a keyboard key press during PIE. "
                "Use UE key names: W, A, S, D, SpaceBar, E, H, Escape, LeftShift, Enter, etc. "
                "Actions: 'tap' (press+release), 'press' (hold down), 'release' (let go). "
                "For movement, use 'press' to start walking and 'release' to stop. "
                "For interactions, use 'tap'."
            ),
            inputSchema={
                "type": "object",
                "properties": {
                    "key": {
                        "type": "string",
                        "description": "UE key name (e.g. 'W', 'SpaceBar', 'E', 'LeftShift', 'Escape').",
                    },
                    "action": {
                        "type": "string",
                        "enum": ["tap", "press", "release"],
                        "description": "Input action. Default: 'tap'.",
                    },
                    "hold_duration_ms": {
                        "type": "number",
                        "description": "For 'tap', how long to hold before release (ms). Default: 100.",
                    },
                },
                "required": ["key"],
            },
        ),
        Tool(
            name="mouse_look",
            description=(
                "Move the camera by applying a mouse look delta during PIE. "
                "delta_x controls yaw (left/right), delta_y controls pitch (up/down). "
                "Positive delta_x turns right, positive delta_y looks up. "
                "Values around 1-5 are subtle, 10-30 are moderate turns, 90+ are large turns."
            ),
            inputSchema={
                "type": "object",
                "properties": {
                    "delta_x": {
                        "type": "number",
                        "description": "Yaw delta (left/right). Positive = turn right.",
                    },
                    "delta_y": {
                        "type": "number",
                        "description": "Pitch delta (up/down). Positive = look up.",
                    },
                },
                "required": ["delta_x", "delta_y"],
            },
        ),
        Tool(
            name="mouse_click",
            description=(
                "Simulate a mouse button click during PIE. "
                "Useful for interacting with UI or in-game click targets."
            ),
            inputSchema={
                "type": "object",
                "properties": {
                    "button": {
                        "type": "string",
                        "enum": ["left", "right", "middle"],
                        "description": "Mouse button. Default: 'left'.",
                    },
                    "action": {
                        "type": "string",
                        "enum": ["click", "press", "release"],
                        "description": "Click action. Default: 'click'.",
                    },
                },
            },
        ),
        Tool(
            name="get_game_state",
            description=(
                "Query the current game state during PIE. "
                "Returns player position, rotation, speed, nearby actors, and play time. "
                "Use this to understand what's happening in the game."
            ),
            inputSchema={
                "type": "object",
                "properties": {},
            },
        ),
        Tool(
            name="call_component_function",
            description=(
                "Call any UFunction on a component by name during PIE or in the editor. "
                "Finds the actor, finds the matching component by class name, and calls the "
                "function via reflection. Supports string, int, float, double, bool, and FName "
                "parameter types."
            ),
            inputSchema={
                "type": "object",
                "properties": {
                    "actor_name": {
                        "type": "string",
                        "description": "Name or label of the target actor.",
                    },
                    "component_class": {
                        "type": "string",
                        "description": "Class name of the component (e.g. StaticMeshComponent).",
                    },
                    "function_name": {
                        "type": "string",
                        "description": "Name of the UFunction to call.",
                    },
                    "args": {
                        "type": "object",
                        "description": "Optional key-value pairs for function parameters.",
                    },
                },
                "required": ["actor_name", "component_class", "function_name"],
            },
        ),
    ]


def get_handlers() -> dict:
    return {
        "start_pie": handle_start_pie,
        "stop_pie": handle_stop_pie,
        "get_pie_status": handle_get_pie_status,
        "press_key": handle_press_key,
        "mouse_look": handle_mouse_look,
        "mouse_click": handle_mouse_click,
        "get_game_state": handle_get_game_state,
        "call_component_function": handle_call_component_function,
    }


async def handle_start_pie(args: dict) -> str:
    resp = await bridge.send_command("start_pie", {})
    if "error" in resp:
        return f"Error: {resp['error']}"
    return f"PIE session requested. Use get_pie_status to confirm it started."


async def handle_stop_pie(args: dict) -> str:
    resp = await bridge.send_command("stop_pie", {})
    if "error" in resp:
        return f"Error: {resp['error']}"
    return "PIE session stop requested."


async def handle_get_pie_status(args: dict) -> str:
    resp = await bridge.send_command("get_pie_status", {})
    if "error" in resp:
        return f"Error: {resp['error']}"

    if not resp.get("is_playing"):
        return "PIE is not running."

    lines = [
        "PIE is running.",
        f"Play time: {resp.get('play_time', 0):.1f}s",
    ]
    if "player_x" in resp:
        lines.append(
            f"Player at: ({resp['player_x']:.0f}, {resp['player_y']:.0f}, {resp['player_z']:.0f})"
        )
    return "\n".join(lines)


async def handle_press_key(args: dict) -> str:
    params = {"key": args["key"]}
    if "action" in args:
        params["action"] = args["action"]
    if "hold_duration_ms" in args:
        params["hold_duration_ms"] = args["hold_duration_ms"]

    resp = await bridge.send_command("key_input", params)
    if "error" in resp:
        return f"Error: {resp['error']}"
    return f"Key '{resp.get('key')}' {resp.get('action')}."


async def handle_mouse_look(args: dict) -> str:
    params = {"delta_x": args["delta_x"], "delta_y": args["delta_y"]}
    resp = await bridge.send_command("mouse_move", params)
    if "error" in resp:
        return f"Error: {resp['error']}"
    return f"Mouse moved: dx={resp.get('delta_x')}, dy={resp.get('delta_y')}"


async def handle_mouse_click(args: dict) -> str:
    params = {}
    if "button" in args:
        params["button"] = args["button"]
    if "action" in args:
        params["action"] = args["action"]

    resp = await bridge.send_command("mouse_click", params)
    if "error" in resp:
        return f"Error: {resp['error']}"
    return f"Mouse {resp.get('button')} {resp.get('action')}."


async def handle_get_game_state(args: dict) -> str:
    resp = await bridge.send_command("get_game_state", {})
    if "error" in resp:
        return f"Error: {resp['error']}"

    lines = [f"Play time: {resp.get('play_time', 0):.1f}s"]

    player = resp.get("player")
    if player:
        lines.append(
            f"\nPlayer:"
            f"\n  Position: ({player.get('x', 0):.0f}, {player.get('y', 0):.0f}, {player.get('z', 0):.0f})"
            f"\n  Rotation: yaw={player.get('yaw', 0):.1f}, pitch={player.get('pitch', 0):.1f}"
            f"\n  Speed: {player.get('speed', 0):.0f}"
            f"\n  Moving: {'yes' if player.get('is_moving') else 'no'}"
        )

    actors = resp.get("nearby_actors", [])
    if actors:
        lines.append(f"\nNearby actors ({resp.get('nearby_actor_count', 0)}):")
        for a in actors[:15]:
            lines.append(
                f"  - {a['name']} ({a['class']}) [{a.get('distance', 0):.0f} units]"
            )
        if len(actors) > 15:
            lines.append(f"  ... and {len(actors) - 15} more")

    return "\n".join(lines)


async def handle_call_component_function(args: dict) -> str:
    params = {
        "actor_name": args["actor_name"],
        "component_class": args["component_class"],
        "function_name": args["function_name"],
    }
    if "args" in args:
        params["args"] = args["args"]

    resp = await bridge.send_command("call_component_function", params)
    if "error" in resp:
        return f"Error: {resp['error']}"

    result = resp.get("return_value", "void")
    return f"Called {args['function_name']} on {args['component_class']} of {args['actor_name']}. Result: {result}"

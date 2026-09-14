# Copyright (c) 2026 victorvksy. All rights reserved.

"""Actor tools — spawn, delete, move, and inspect actors in the UE5 editor level."""

from mcp.types import Tool

from unreal_mcp.bridge import bridge


def get_tools() -> list[Tool]:
    return [
        Tool(
            name="list_actors",
            description=(
                "List all actors in the currently open Unreal Editor level. "
                "Returns each actor's name, class, and position (x, y, z). "
                "Use class_filter to show only actors whose class name contains the filter string."
            ),
            inputSchema={
                "type": "object",
                "properties": {
                    "class_filter": {
                        "type": "string",
                        "description": "Optional substring to filter actor class names (e.g. 'PointLight', 'StaticMesh').",
                    },
                },
            },
        ),
        Tool(
            name="spawn_actor",
            description=(
                "Spawn a new actor in the editor level at a specific position. "
                "Accepts native UE5 class names (e.g. 'PointLight', 'StaticMeshActor') "
                "or Blueprint asset names (e.g. 'BP_MyCharacter' or full path '/Game/Blueprints/BP_MyCharacter'). "
                "Returns the spawned actor's name."
            ),
            inputSchema={
                "type": "object",
                "properties": {
                    "class_name": {
                        "type": "string",
                        "description": "UE5 class name or Blueprint name to spawn.",
                    },
                    "x": {"type": "number", "description": "X position in world space."},
                    "y": {"type": "number", "description": "Y position in world space."},
                    "z": {"type": "number", "description": "Z position in world space."},
                    "pitch": {"type": "number", "description": "Pitch rotation in degrees (optional)."},
                    "yaw": {"type": "number", "description": "Yaw rotation in degrees (optional)."},
                    "roll": {"type": "number", "description": "Roll rotation in degrees (optional)."},
                },
                "required": ["class_name", "x", "y", "z"],
            },
        ),
        Tool(
            name="delete_actor",
            description=(
                "Delete an actor from the editor level by its name. "
                "Use list_actors first to find the exact actor name."
            ),
            inputSchema={
                "type": "object",
                "properties": {
                    "name": {
                        "type": "string",
                        "description": "The actor's display name or internal name.",
                    },
                },
                "required": ["name"],
            },
        ),
        Tool(
            name="set_actor_transform",
            description=(
                "Set the position, rotation, and/or scale of an actor by name. "
                "You can set any combination — only provided fields are changed."
            ),
            inputSchema={
                "type": "object",
                "properties": {
                    "name": {
                        "type": "string",
                        "description": "The actor's name.",
                    },
                    "position": {
                        "type": "object",
                        "properties": {
                            "x": {"type": "number"},
                            "y": {"type": "number"},
                            "z": {"type": "number"},
                        },
                        "description": "New world position. All three components required if provided.",
                    },
                    "rotation": {
                        "type": "object",
                        "properties": {
                            "pitch": {"type": "number"},
                            "yaw": {"type": "number"},
                            "roll": {"type": "number"},
                        },
                        "description": "New rotation in degrees.",
                    },
                    "scale": {
                        "type": "object",
                        "properties": {
                            "x": {"type": "number"},
                            "y": {"type": "number"},
                            "z": {"type": "number"},
                        },
                        "description": "New scale.",
                    },
                },
                "required": ["name"],
            },
        ),
        Tool(
            name="get_actor_property",
            description=(
                "Read any property from an actor using UE5 reflection. "
                "Use 'ComponentName.PropertyName' syntax to read component properties. "
                "Returns the property value as a string and its C++ type."
            ),
            inputSchema={
                "type": "object",
                "properties": {
                    "actor_name": {
                        "type": "string",
                        "description": "The actor's name.",
                    },
                    "property_name": {
                        "type": "string",
                        "description": "Property name, e.g. 'Mobility' or 'LightComponent0.Intensity'.",
                    },
                },
                "required": ["actor_name", "property_name"],
            },
        ),
        Tool(
            name="set_actor_property",
            description=(
                "Set any property on an actor using UE5 reflection. "
                "Use 'ComponentName.PropertyName' syntax for component properties. "
                "Value must be a string in UE5's text export format."
            ),
            inputSchema={
                "type": "object",
                "properties": {
                    "actor_name": {
                        "type": "string",
                        "description": "The actor's name.",
                    },
                    "property_name": {
                        "type": "string",
                        "description": "Property name, e.g. 'Mobility' or 'LightComponent0.Intensity'.",
                    },
                    "value": {
                        "type": "string",
                        "description": "New value as a string (UE5 text format).",
                    },
                },
                "required": ["actor_name", "property_name", "value"],
            },
        ),
    ]


def get_handlers() -> dict:
    return {
        "list_actors": handle_list_actors,
        "spawn_actor": handle_spawn_actor,
        "delete_actor": handle_delete_actor,
        "set_actor_transform": handle_set_actor_transform,
        "get_actor_property": handle_get_actor_property,
        "set_actor_property": handle_set_actor_property,
    }


async def handle_list_actors(args: dict) -> str:
    resp = await bridge.send_command("list_actors", {
        k: v for k, v in {"class_filter": args.get("class_filter")}.items() if v
    })

    if "error" in resp:
        return f"Error: {resp['error']}"

    actors = resp.get("actors", [])
    if not actors:
        filter_msg = f" matching '{args.get('class_filter')}'" if args.get("class_filter") else ""
        return f"No actors found{filter_msg} in the current level."

    lines = [f"Found {resp['count']} actors:\n"]
    for a in actors:
        lines.append(
            f"  - {a['name']} ({a['class']}) at ({a['x']:.1f}, {a['y']:.1f}, {a['z']:.1f})"
        )
    return "\n".join(lines)


async def handle_spawn_actor(args: dict) -> str:
    resp = await bridge.send_command("spawn_actor", args)

    if "error" in resp:
        return f"Error spawning actor: {resp['error']}"

    return (
        f"Spawned {resp['class']} as '{resp['name']}' "
        f"(internal: {resp['internal_name']})"
    )


async def handle_delete_actor(args: dict) -> str:
    resp = await bridge.send_command("delete_actor", {"name": args["name"]})

    if "error" in resp:
        return f"Error deleting actor: {resp['error']}"

    return f"Deleted actor: {resp['deleted']}"


async def handle_set_actor_transform(args: dict) -> str:
    params = {"name": args["name"]}
    if "position" in args:
        params["position"] = args["position"]
    if "rotation" in args:
        params["rotation"] = args["rotation"]
    if "scale" in args:
        params["scale"] = args["scale"]

    resp = await bridge.send_command("set_actor_transform", params)

    if "error" in resp:
        return f"Error setting transform: {resp['error']}"

    pos = resp.get("position", {})
    rot = resp.get("rotation", {})
    scl = resp.get("scale", {})
    return (
        f"Updated transform for '{resp['name']}':\n"
        f"  Position: ({pos.get('x', 0):.1f}, {pos.get('y', 0):.1f}, {pos.get('z', 0):.1f})\n"
        f"  Rotation: (pitch={rot.get('pitch', 0):.1f}, yaw={rot.get('yaw', 0):.1f}, roll={rot.get('roll', 0):.1f})\n"
        f"  Scale: ({scl.get('x', 1):.1f}, {scl.get('y', 1):.1f}, {scl.get('z', 1):.1f})"
    )


async def handle_get_actor_property(args: dict) -> str:
    resp = await bridge.send_command("get_property", {
        "actor_name": args["actor_name"],
        "property_name": args["property_name"],
    })

    if "error" in resp:
        return f"Error reading property: {resp['error']}"

    return (
        f"{resp['actor']}.{resp['property']} = {resp['value']}\n"
        f"  (type: {resp['type']})"
    )


async def handle_set_actor_property(args: dict) -> str:
    resp = await bridge.send_command("set_property", {
        "actor_name": args["actor_name"],
        "property_name": args["property_name"],
        "value": args["value"],
    })

    if "error" in resp:
        return f"Error setting property: {resp['error']}"

    return f"Set {resp['actor']}.{resp['property']} = {resp['new_value']}"

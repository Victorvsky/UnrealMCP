# Copyright (c) 2026 victorvksy. All rights reserved.

"""Level tools — query level info, find actors by class or within a radius."""

from mcp.types import Tool

from unreal_mcp.bridge import bridge


def get_tools() -> list[Tool]:
    return [
        Tool(
            name="get_level_info",
            description=(
                "Get information about the currently open level: "
                "level name, full path, total actor count, and world bounds."
            ),
            inputSchema={
                "type": "object",
                "properties": {},
            },
        ),
        Tool(
            name="find_actors_by_class",
            description=(
                "Find all actors of a specific class in the current level. "
                "Uses substring matching on class names, so 'PointLight' matches "
                "'PointLight', 'PointLightComponent', etc."
            ),
            inputSchema={
                "type": "object",
                "properties": {
                    "class_name": {
                        "type": "string",
                        "description": "Class name to search for (substring match).",
                    },
                },
                "required": ["class_name"],
            },
        ),
        Tool(
            name="screenshot",
            description=(
                "Take a screenshot of the active editor viewport and save it as a PNG file. "
                "Returns the file path to the saved screenshot. "
                "Use this to capture the current editor view."
            ),
            inputSchema={
                "type": "object",
                "properties": {
                    "filename": {
                        "type": "string",
                        "description": "Optional filename for the screenshot (e.g. 'my_shot.png'). Auto-generated if omitted.",
                    },
                },
            },
        ),
        Tool(
            name="find_actors_in_radius",
            description=(
                "Find all actors within a given radius of a point in world space. "
                "Returns actors sorted by distance with their positions."
            ),
            inputSchema={
                "type": "object",
                "properties": {
                    "x": {"type": "number", "description": "Center X position."},
                    "y": {"type": "number", "description": "Center Y position."},
                    "z": {"type": "number", "description": "Center Z position."},
                    "radius": {"type": "number", "description": "Search radius in Unreal units (cm)."},
                    "class_filter": {
                        "type": "string",
                        "description": "Optional class name filter (substring match).",
                    },
                },
                "required": ["x", "y", "z", "radius"],
            },
        ),
    ]


def get_handlers() -> dict:
    return {
        "get_level_info": handle_get_level_info,
        "screenshot": handle_screenshot,
        "find_actors_by_class": handle_find_actors_by_class,
        "find_actors_in_radius": handle_find_actors_in_radius,
    }


async def handle_screenshot(args: dict) -> str:
    params = {}
    if "filename" in args:
        params["filename"] = args["filename"]

    resp = await bridge.send_command("screenshot", params)

    if "error" in resp:
        return f"Error: {resp['error']}"

    return (
        f"Screenshot saved!\n"
        f"Path: {resp['path']}\n"
        f"Size: {resp['width']}x{resp['height']}"
    )


async def handle_get_level_info(args: dict) -> str:
    resp = await bridge.send_command("level_info", {})

    if "error" in resp:
        return f"Error: {resp['error']}"

    lines = [
        f"Level: {resp['level_name']}",
        f"Path: {resp['level_path']}",
        f"Actor count: {resp['actor_count']}",
    ]

    bounds = resp.get("world_bounds")
    if bounds:
        lines.append(
            f"World bounds: "
            f"({bounds['min_x']:.0f}, {bounds['min_y']:.0f}, {bounds['min_z']:.0f}) to "
            f"({bounds['max_x']:.0f}, {bounds['max_y']:.0f}, {bounds['max_z']:.0f})"
        )

    return "\n".join(lines)


async def handle_find_actors_by_class(args: dict) -> str:
    resp = await bridge.send_command("find_by_class", {"class_name": args["class_name"]})

    if "error" in resp:
        return f"Error: {resp['error']}"

    actors = resp.get("actors", [])
    if not actors:
        return f"No actors found matching class '{args['class_name']}'."

    lines = [f"Found {resp['count']} actors matching '{resp['class_filter']}':\n"]
    for a in actors:
        lines.append(
            f"  - {a['name']} ({a['class']}) at ({a['x']:.1f}, {a['y']:.1f}, {a['z']:.1f})"
        )
    return "\n".join(lines)


async def handle_find_actors_in_radius(args: dict) -> str:
    params = {
        "x": args["x"],
        "y": args["y"],
        "z": args["z"],
        "radius": args["radius"],
    }
    if "class_filter" in args:
        params["class_filter"] = args["class_filter"]

    resp = await bridge.send_command("find_in_radius", params)

    if "error" in resp:
        return f"Error: {resp['error']}"

    actors = resp.get("actors", [])
    if not actors:
        return f"No actors found within {resp['radius']:.0f} units of ({resp['center_x']:.0f}, {resp['center_y']:.0f}, {resp['center_z']:.0f})."

    # Sort by distance
    actors.sort(key=lambda a: a.get("distance", 0))

    lines = [
        f"Found {resp['count']} actors within {resp['radius']:.0f} units "
        f"of ({resp['center_x']:.0f}, {resp['center_y']:.0f}, {resp['center_z']:.0f}):\n"
    ]
    for a in actors:
        lines.append(
            f"  - {a['name']} ({a['class']}) "
            f"at ({a['x']:.1f}, {a['y']:.1f}, {a['z']:.1f}) "
            f"[{a['distance']:.1f} units away]"
        )
    return "\n".join(lines)

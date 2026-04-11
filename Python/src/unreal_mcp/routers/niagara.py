"""Niagara particle system tools — list, read, configure emitters and parameters."""

from mcp.types import Tool

from unreal_mcp.bridge import bridge


def get_tools() -> list[Tool]:
    return [
        Tool(
            name="list_niagara_systems",
            description=(
                "List all Niagara particle system assets in the project. "
                "Returns name, path, and emitter count for each system."
            ),
            inputSchema={
                "type": "object",
                "properties": {
                    "path_filter": {
                        "type": "string",
                        "description": "Substring to filter system paths.",
                    },
                },
            },
        ),
        Tool(
            name="read_niagara_system",
            description=(
                "Read detailed information about a Niagara system: emitters, "
                "user parameters, simulation targets. Use this to understand "
                "the system structure before modifying it."
            ),
            inputSchema={
                "type": "object",
                "properties": {
                    "system": {
                        "type": "string",
                        "description": "Niagara system name or full asset path.",
                    },
                },
                "required": ["system"],
            },
        ),
        Tool(
            name="get_niagara_emitter_properties",
            description=(
                "Get detailed properties of a specific emitter within a Niagara system, "
                "including scripts and simulation target."
            ),
            inputSchema={
                "type": "object",
                "properties": {
                    "system": {
                        "type": "string",
                        "description": "Niagara system name or path.",
                    },
                    "emitter_name": {
                        "type": "string",
                        "description": "Name of the emitter to inspect.",
                    },
                    "emitter_index": {
                        "type": "integer",
                        "description": "Index of the emitter (alternative to emitter_name).",
                    },
                },
                "required": ["system"],
            },
        ),
        Tool(
            name="set_niagara_parameter",
            description=(
                "Set a user-exposed parameter on a Niagara system. "
                "Supports float, int32, bool, Vector ({x,y,z}), and LinearColor ({r,g,b,a}) types."
            ),
            inputSchema={
                "type": "object",
                "properties": {
                    "system": {
                        "type": "string",
                        "description": "Niagara system name or path.",
                    },
                    "parameter_name": {
                        "type": "string",
                        "description": "Name of the user parameter to set.",
                    },
                    "value": {
                        "description": (
                            "Value to set. For float/int: number. For bool: true/false. "
                            "For Vector: {x, y, z}. For Color: {r, g, b, a}."
                        ),
                    },
                },
                "required": ["system", "parameter_name", "value"],
            },
        ),
        Tool(
            name="set_niagara_emitter_enabled",
            description="Enable or disable a specific emitter within a Niagara system.",
            inputSchema={
                "type": "object",
                "properties": {
                    "system": {
                        "type": "string",
                        "description": "Niagara system name or path.",
                    },
                    "emitter_name": {
                        "type": "string",
                        "description": "Name of the emitter.",
                    },
                    "emitter_index": {
                        "type": "integer",
                        "description": "Index of the emitter (alternative to emitter_name).",
                    },
                    "enabled": {
                        "type": "boolean",
                        "description": "Whether the emitter should be enabled.",
                    },
                },
                "required": ["system", "enabled"],
            },
        ),
    ]


def get_handlers() -> dict:
    return {
        "list_niagara_systems": handle_list_niagara_systems,
        "read_niagara_system": handle_read_niagara_system,
        "get_niagara_emitter_properties": handle_get_niagara_emitter_properties,
        "set_niagara_parameter": handle_set_niagara_parameter,
        "set_niagara_emitter_enabled": handle_set_niagara_emitter_enabled,
    }


async def handle_list_niagara_systems(args: dict) -> str:
    params = {}
    if "path_filter" in args:
        params["path_filter"] = args["path_filter"]

    resp = await bridge.send_command("list_niagara_systems", params)
    if "error" in resp:
        return f"Error: {resp['error']}"

    systems = resp.get("systems", [])
    if not systems:
        return "No Niagara systems found."

    lines = [f"Found {resp['count']} Niagara systems:\n"]
    for sys in systems:
        emitters = sys.get("emitter_count", "?")
        lines.append(f"  - {sys['name']} ({emitters} emitters)")
        lines.append(f"    {sys['path']}")
    return "\n".join(lines)


async def handle_read_niagara_system(args: dict) -> str:
    resp = await bridge.send_command("read_niagara_system", {"system": args["system"]})
    if "error" in resp:
        return f"Error: {resp['error']}"

    lines = [
        f"Niagara System: {resp.get('name', '')}",
        f"Path: {resp.get('path', '')}",
    ]

    # Emitters
    emitters = resp.get("emitters", [])
    lines.append(f"\nEmitters ({len(emitters)}):")
    for em in emitters:
        status = "enabled" if em.get("enabled", True) else "DISABLED"
        target = em.get("sim_target", "?")
        lines.append(f"  [{em['index']}] {em['name']} ({target}, {status})")

    # User parameters
    params = resp.get("user_parameters", [])
    if params:
        lines.append(f"\nUser Parameters ({len(params)}):")
        for p in params:
            lines.append(f"  - {p['name']}: {p['type']}")

    return "\n".join(lines)


async def handle_get_niagara_emitter_properties(args: dict) -> str:
    params = {"system": args["system"]}
    if "emitter_name" in args:
        params["emitter_name"] = args["emitter_name"]
    if "emitter_index" in args:
        params["emitter_index"] = args["emitter_index"]

    resp = await bridge.send_command("get_niagara_emitter_properties", params)
    if "error" in resp:
        return f"Error: {resp['error']}"

    status = "enabled" if resp.get("enabled", True) else "DISABLED"
    lines = [
        f"Emitter: {resp.get('name', '')} (index {resp.get('index', '?')})",
        f"Status: {status}",
        f"Sim Target: {resp.get('sim_target', '?')}",
    ]

    scripts = resp.get("scripts", [])
    if scripts:
        lines.append(f"\nScripts ({len(scripts)}):")
        for s in scripts:
            lines.append(f"  - {s['purpose']}: {s['name']}")

    return "\n".join(lines)


async def handle_set_niagara_parameter(args: dict) -> str:
    params = {
        "system": args["system"],
        "parameter_name": args["parameter_name"],
        "value": args["value"],
    }

    resp = await bridge.send_command("set_niagara_parameter", params)
    if "error" in resp:
        return f"Error: {resp['error']}"

    return (
        f"Set parameter '{resp.get('parameter', '')}' ({resp.get('type', '')}) "
        f"on system '{resp.get('system', '')}'"
    )


async def handle_set_niagara_emitter_enabled(args: dict) -> str:
    params = {"system": args["system"], "enabled": args["enabled"]}
    if "emitter_name" in args:
        params["emitter_name"] = args["emitter_name"]
    if "emitter_index" in args:
        params["emitter_index"] = args["emitter_index"]

    resp = await bridge.send_command("set_niagara_emitter_enabled", params)
    if "error" in resp:
        return f"Error: {resp['error']}"

    state = "enabled" if resp.get("enabled", True) else "disabled"
    return f"Emitter '{resp.get('emitter', '')}' {state} on system '{resp.get('system', '')}'"

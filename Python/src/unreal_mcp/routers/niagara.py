# Copyright (c) 2026 victorvksy. All rights reserved.

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
        Tool(
            name="get_niagara_emitter_modules",
            description=(
                "List all modules in a Niagara emitter and their rapid iteration parameters "
                "with current values. Use this to discover what inputs can be modified with "
                "set_niagara_module_input."
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
            name="set_niagara_module_input",
            description=(
                "Set a rapid iteration parameter (module input) on a Niagara emitter. "
                "Use get_niagara_emitter_modules first to discover available parameter names. "
                "Supports float, int32, bool, Vector ({x,y,z}), and LinearColor ({r,g,b,a})."
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
                        "description": "Name of the emitter.",
                    },
                    "emitter_index": {
                        "type": "integer",
                        "description": "Index of the emitter (alternative to emitter_name).",
                    },
                    "parameter_name": {
                        "type": "string",
                        "description": "RI parameter name (substring match). Use get_niagara_emitter_modules to find names.",
                    },
                    "script": {
                        "type": "string",
                        "description": "Optional script filter: 'Spawn', 'Update', 'EmitterSpawn', 'EmitterUpdate'.",
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
    ]


def get_handlers() -> dict:
    return {
        "list_niagara_systems": handle_list_niagara_systems,
        "read_niagara_system": handle_read_niagara_system,
        "get_niagara_emitter_properties": handle_get_niagara_emitter_properties,
        "set_niagara_parameter": handle_set_niagara_parameter,
        "set_niagara_emitter_enabled": handle_set_niagara_emitter_enabled,
        "get_niagara_emitter_modules": handle_get_niagara_emitter_modules,
        "set_niagara_module_input": handle_set_niagara_module_input,
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


async def handle_get_niagara_emitter_modules(args: dict) -> str:
    params = {"system": args["system"]}
    if "emitter_name" in args:
        params["emitter_name"] = args["emitter_name"]
    if "emitter_index" in args:
        params["emitter_index"] = args["emitter_index"]

    resp = await bridge.send_command("get_niagara_emitter_modules", params)
    if "error" in resp:
        return f"Error: {resp['error']}"

    lines = [f"Emitter: {resp.get('emitter', '')} (index {resp.get('index', '?')})"]

    # Modules
    modules = resp.get("modules", [])
    if modules:
        lines.append(f"\nModules ({len(modules)}):")
        for m in modules:
            usage = m.get("usage", "?")
            lines.append(f"  [{usage}] {m['name']}")
            for inp in m.get("inputs", []):
                default = f" = {inp['default']}" if "default" in inp else ""
                lines.append(f"    - {inp['name']} ({inp['type']}){default}")

    # RI Parameters
    ri_params = resp.get("rapid_iteration_parameters", [])
    if ri_params:
        lines.append(f"\nRapid Iteration Parameters ({len(ri_params)}):")
        for p in ri_params:
            val = p.get("value", "?")
            if isinstance(val, dict):
                if "r" in val:
                    val = f"({val['r']:.3f}, {val['g']:.3f}, {val['b']:.3f}, {val.get('a', 1):.3f})"
                else:
                    val = f"({val.get('x', 0):.1f}, {val.get('y', 0):.1f}, {val.get('z', 0):.1f})"
            lines.append(f"  [{p.get('script', '?')}] {p['name']} ({p['type']}) = {val}")

    return "\n".join(lines)


async def handle_set_niagara_module_input(args: dict) -> str:
    params = {
        "system": args["system"],
        "parameter_name": args["parameter_name"],
        "value": args["value"],
    }
    if "emitter_name" in args:
        params["emitter_name"] = args["emitter_name"]
    if "emitter_index" in args:
        params["emitter_index"] = args["emitter_index"]
    if "script" in args:
        params["script"] = args["script"]

    resp = await bridge.send_command("set_niagara_module_input", params)
    if "error" in resp:
        return f"Error: {resp['error']}"

    return (
        f"Set '{resp.get('parameter', '')}' ({resp.get('type', '')}) "
        f"on emitter '{resp.get('emitter', '')}' [{resp.get('script', '')}] "
        f"in system '{resp.get('system', '')}'"
    )

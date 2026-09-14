# Copyright (c) 2026 victorvksy. All rights reserved.

"""Material tools — read and modify Material Instance parameters in the UE5 editor."""

from mcp.types import Tool

from unreal_mcp.bridge import bridge


def get_tools() -> list[Tool]:
    return [
        Tool(
            name="get_material_params",
            description=(
                "Read all parameters from a Material Instance (scalar, vector, texture, static switch). "
                "Returns parameter names, current values, types, and whether each is overridden. "
                "Vector params include both linear color and sRGB hex values."
            ),
            inputSchema={
                "type": "object",
                "properties": {
                    "asset_path": {
                        "type": "string",
                        "description": (
                            "Full asset path to the material instance, e.g. "
                            "'/Game/MyFolder/MI_MyMaterial.MI_MyMaterial'"
                        ),
                    },
                },
                "required": ["asset_path"],
            },
        ),
        Tool(
            name="set_material_param",
            description=(
                "Set a parameter on a Material Instance. Supports scalar (float), "
                "vector (color as hex '#FF8800' or UE format '(R=1.0,G=0.5,B=0.0,A=1.0)'), "
                "and texture (asset path) parameters."
            ),
            inputSchema={
                "type": "object",
                "properties": {
                    "asset_path": {
                        "type": "string",
                        "description": "Full asset path to the material instance.",
                    },
                    "param_name": {
                        "type": "string",
                        "description": "Name of the parameter to set.",
                    },
                    "param_type": {
                        "type": "string",
                        "enum": ["scalar", "vector", "texture"],
                        "description": "Type of parameter: scalar, vector, or texture.",
                    },
                    "value": {
                        "description": (
                            "New value. For scalar: a number. For vector: hex string '#FF8800' "
                            "or UE format '(R=1.0,G=0.5,B=0.0,A=1.0)'. For texture: asset path."
                        ),
                    },
                },
                "required": ["asset_path", "param_name", "param_type", "value"],
            },
        ),
    ]


def get_handlers() -> dict:
    return {
        "get_material_params": handle_get_material_params,
        "set_material_param": handle_set_material_param,
    }


async def handle_get_material_params(args: dict) -> str:
    resp = await bridge.send_command("get_material_params", {
        "asset_path": args["asset_path"],
    })

    if "error" in resp:
        return f"Error: {resp['error']}"

    lines = [
        f"Material: {resp['name']}",
        f"Path: {resp['path']}",
        f"Parent: {resp.get('parent', 'None')}",
        "",
    ]

    scalars = resp.get("scalar_params", [])
    if scalars:
        lines.append("Scalar Parameters:")
        for p in scalars:
            override = " [OVERRIDDEN]" if p.get("overridden") else ""
            lines.append(f"  - {p['name']}: {p['value']}{override}")
        lines.append("")

    vectors = resp.get("vector_params", [])
    if vectors:
        lines.append("Vector Parameters:")
        for p in vectors:
            override = " [OVERRIDDEN]" if p.get("overridden") else ""
            lines.append(f"  - {p['name']}: {p['value']} ({p.get('hex', '?')}){override}")
        lines.append("")

    textures = resp.get("texture_params", [])
    if textures:
        lines.append("Texture Parameters:")
        for p in textures:
            override = " [OVERRIDDEN]" if p.get("overridden") else ""
            lines.append(f"  - {p['name']}: {p['value']}{override}")
        lines.append("")

    switches = resp.get("static_switch_params", [])
    if switches:
        lines.append("Static Switch Parameters:")
        for p in switches:
            override = " [OVERRIDDEN]" if p.get("overridden") else ""
            lines.append(f"  - {p['name']}: {p['value']}{override}")
        lines.append("")

    return "\n".join(lines)


async def handle_set_material_param(args: dict) -> str:
    params = {
        "asset_path": args["asset_path"],
        "param_name": args["param_name"],
        "param_type": args["param_type"],
    }

    # Value can be number or string depending on type
    value = args["value"]
    if args["param_type"] == "scalar":
        params["value"] = float(value)
    else:
        params["value"] = str(value)

    resp = await bridge.send_command("set_material_param", params)

    if "error" in resp:
        return f"Error: {resp['error']}"

    return f"Set {resp['material']}.{resp['param']} ({resp['type']})"

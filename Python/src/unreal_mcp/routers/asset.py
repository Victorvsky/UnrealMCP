# Copyright (c) 2026 victorvksy. All rights reserved.

"""Asset tools — import files, create material instances, and read/write any asset property."""

from mcp.types import Tool

from unreal_mcp.bridge import bridge


def get_tools() -> list[Tool]:
    return [
        Tool(
            name="import_asset",
            description=(
                "Import an external file (texture, mesh, sound, etc.) into the UE5 project. "
                "Supports any file type that Unreal's import pipeline handles: "
                ".png/.jpg/.tga (textures), .fbx/.obj (meshes), .wav (audio), etc."
            ),
            inputSchema={
                "type": "object",
                "properties": {
                    "file_path": {
                        "type": "string",
                        "description": "Absolute path to the file on disk.",
                    },
                    "dest_path": {
                        "type": "string",
                        "description": "Destination content folder (e.g. '/Game/Textures/'). Default: '/Game/Imported/'.",
                    },
                },
                "required": ["file_path"],
            },
        ),
        Tool(
            name="create_material_instance",
            description=(
                "Create a new Material Instance from an existing parent material. "
                "After creation, use set_material_param to configure parameters."
            ),
            inputSchema={
                "type": "object",
                "properties": {
                    "parent_material": {
                        "type": "string",
                        "description": "Full path to the parent Material or Material Instance.",
                    },
                    "name": {
                        "type": "string",
                        "description": "Name for the new material instance.",
                    },
                    "path": {
                        "type": "string",
                        "description": "Destination folder path. Default: '/Game/Materials/'.",
                    },
                },
                "required": ["parent_material", "name"],
            },
        ),
        Tool(
            name="get_asset_property",
            description=(
                "Read any property from a UE5 asset using reflection. Works on any asset type "
                "(SoundAttenuation, SoundCue, DataAsset, CurveFloat, etc.). "
                "Supports dot-separated paths for nested struct properties (e.g. 'Attenuation.FalloffDistance'). "
                "If a property is not found, returns a list of available properties. "
                "If the property is a struct, also returns its sub-properties for further exploration."
            ),
            inputSchema={
                "type": "object",
                "properties": {
                    "asset_path": {
                        "type": "string",
                        "description": (
                            "Full asset path including object name, e.g. "
                            "'/Game/Audio/SA_Wisp3D.SA_Wisp3D'"
                        ),
                    },
                    "property_name": {
                        "type": "string",
                        "description": (
                            "Property name. Use dot notation for nested structs, e.g. "
                            "'Attenuation.FalloffDistance' or 'Attenuation.bAttenuate'."
                        ),
                    },
                },
                "required": ["asset_path", "property_name"],
            },
        ),
        Tool(
            name="set_asset_property",
            description=(
                "Set any property on a UE5 asset using reflection. Works on any asset type. "
                "Supports dot-separated paths for nested struct properties. "
                "Value must be in UE5 text export format (same format returned by get_asset_property)."
            ),
            inputSchema={
                "type": "object",
                "properties": {
                    "asset_path": {
                        "type": "string",
                        "description": "Full asset path including object name.",
                    },
                    "property_name": {
                        "type": "string",
                        "description": "Property name (dot notation for nested structs).",
                    },
                    "value": {
                        "type": "string",
                        "description": (
                            "New value in UE5 text format. Examples: '2000.0' for float, "
                            "'True' for bool, 'Logarithmic' for enum, "
                            "'(R=1.0,G=0.5,B=0.0,A=1.0)' for color."
                        ),
                    },
                },
                "required": ["asset_path", "property_name", "value"],
            },
        ),
    ]


def get_handlers() -> dict:
    return {
        "import_asset": handle_import_asset,
        "create_material_instance": handle_create_material_instance,
        "get_asset_property": handle_get_asset_property,
        "set_asset_property": handle_set_asset_property,
    }


async def handle_import_asset(args: dict) -> str:
    params = {"file_path": args["file_path"]}
    if "dest_path" in args:
        params["dest_path"] = args["dest_path"]

    resp = await bridge.send_command("import_asset", params)
    if "error" in resp:
        return f"Error: {resp['error']}"

    imported = resp.get("imported", [])
    lines = [f"Imported {resp.get('count', 0)} asset(s):\n"]
    for a in imported:
        lines.append(f"  - {a['name']} ({a['class']})")
        lines.append(f"    Path: {a['path']}")
    return "\n".join(lines)


async def handle_create_material_instance(args: dict) -> str:
    params = {
        "parent_material": args["parent_material"],
        "name": args["name"],
    }
    if "path" in args:
        params["path"] = args["path"]

    resp = await bridge.send_command("create_material_instance", params)
    if "error" in resp:
        return f"Error: {resp['error']}"

    return (
        f"Created material instance '{resp.get('name', '')}'\n"
        f"  Path: {resp.get('path', '')}\n"
        f"  Parent: {resp.get('parent', '')}"
    )


async def handle_get_asset_property(args: dict) -> str:
    resp = await bridge.send_command("get_asset_property", {
        "asset_path": args["asset_path"],
        "property_name": args["property_name"],
    })

    if "error" in resp:
        return f"Error: {resp['error']}"

    lines = [
        f"Asset: {resp.get('asset', '')}",
        f"Property: {resp.get('property', '')}",
        f"Type: {resp.get('type', '')}",
        f"Value: {resp.get('value', '')}",
    ]

    sub_props = resp.get("sub_properties", [])
    if sub_props:
        lines.append("")
        lines.append("Sub-properties (use dot notation to access):")
        for sp in sub_props:
            lines.append(f"  - {sp['name']} ({sp['type']})")

    return "\n".join(lines)


async def handle_set_asset_property(args: dict) -> str:
    resp = await bridge.send_command("set_asset_property", {
        "asset_path": args["asset_path"],
        "property_name": args["property_name"],
        "value": args["value"],
    })

    if "error" in resp:
        return f"Error: {resp['error']}"

    return (
        f"Set {resp.get('asset', '')}.{resp.get('property', '')} = {resp.get('new_value', '')}"
    )

"""Asset tools — import external files and create material instances."""

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
    ]


def get_handlers() -> dict:
    return {
        "import_asset": handle_import_asset,
        "create_material_instance": handle_create_material_instance,
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

# Copyright (c) 2026 victorvksy. All rights reserved.

"""Editor utility tools — save, undo/redo, console commands, asset management."""

from mcp.types import Tool

from unreal_mcp.bridge import bridge


def get_tools() -> list[Tool]:
    return [
        Tool(
            name="execute_console_command",
            description=(
                "Execute an Unreal Engine console command. "
                "Supports any command you'd type in the editor console "
                "(e.g. 'stat fps', 'r.ScreenPercentage 100', 'slomo 0.5')."
            ),
            inputSchema={
                "type": "object",
                "properties": {
                    "command": {
                        "type": "string",
                        "description": "The console command to execute.",
                    },
                },
                "required": ["command"],
            },
        ),
        Tool(
            name="save_asset",
            description="Save a specific asset to disk by path.",
            inputSchema={
                "type": "object",
                "properties": {
                    "asset_path": {
                        "type": "string",
                        "description": "Full asset path (e.g. '/Game/Blueprints/BP_MyActor').",
                    },
                },
                "required": ["asset_path"],
            },
        ),
        Tool(
            name="save_current_level",
            description="Save the currently open level to disk.",
            inputSchema={
                "type": "object",
                "properties": {},
            },
        ),
        Tool(
            name="undo",
            description="Undo the last editor action (Ctrl+Z equivalent).",
            inputSchema={
                "type": "object",
                "properties": {},
            },
        ),
        Tool(
            name="redo",
            description="Redo the last undone editor action (Ctrl+Y equivalent).",
            inputSchema={
                "type": "object",
                "properties": {},
            },
        ),
        Tool(
            name="create_blueprint",
            description=(
                "Create a new empty Blueprint asset. "
                "Specify a parent class (defaults to Actor) and a destination path."
            ),
            inputSchema={
                "type": "object",
                "properties": {
                    "name": {
                        "type": "string",
                        "description": "Name for the new Blueprint (e.g. 'BP_MyNewActor').",
                    },
                    "parent_class": {
                        "type": "string",
                        "description": "Parent class (e.g. 'Actor', 'Character', 'Pawn', 'PlayerController'). Default: 'Actor'.",
                    },
                    "path": {
                        "type": "string",
                        "description": "Destination folder path (e.g. '/Game/Blueprints/'). Default: '/Game/Blueprints/'.",
                    },
                },
                "required": ["name"],
            },
        ),
        Tool(
            name="duplicate_asset",
            description="Duplicate an existing asset to a new location.",
            inputSchema={
                "type": "object",
                "properties": {
                    "source_path": {
                        "type": "string",
                        "description": "Full path of the asset to duplicate.",
                    },
                    "dest_path": {
                        "type": "string",
                        "description": "Destination folder path.",
                    },
                    "dest_name": {
                        "type": "string",
                        "description": "Name for the duplicated asset.",
                    },
                },
                "required": ["source_path", "dest_path", "dest_name"],
            },
        ),
        Tool(
            name="delete_asset",
            description="Delete an asset from the project. Use with caution — check for references first.",
            inputSchema={
                "type": "object",
                "properties": {
                    "asset_path": {
                        "type": "string",
                        "description": "Full path of the asset to delete.",
                    },
                },
                "required": ["asset_path"],
            },
        ),
        Tool(
            name="rename_asset",
            description="Rename an asset and optionally move it to a new path.",
            inputSchema={
                "type": "object",
                "properties": {
                    "source_path": {
                        "type": "string",
                        "description": "Current full path of the asset.",
                    },
                    "new_name": {
                        "type": "string",
                        "description": "New name for the asset.",
                    },
                    "new_path": {
                        "type": "string",
                        "description": "Optional new folder path. Stays in same folder if omitted.",
                    },
                },
                "required": ["source_path", "new_name"],
            },
        ),
        Tool(
            name="open_level",
            description="Open/load a different level in the editor.",
            inputSchema={
                "type": "object",
                "properties": {
                    "level_path": {
                        "type": "string",
                        "description": "Level path (e.g. '/Game/Levels/L_MyLevel') or partial name for search.",
                    },
                },
                "required": ["level_path"],
            },
        ),
        Tool(
            name="list_assets",
            description=(
                "List assets in the project, optionally filtered by path and class. "
                "Useful for browsing the content browser programmatically."
            ),
            inputSchema={
                "type": "object",
                "properties": {
                    "path_filter": {
                        "type": "string",
                        "description": "Package path to search in (e.g. '/Game/Blueprints'). Recursive.",
                    },
                    "class_filter": {
                        "type": "string",
                        "description": "Asset class name to filter by (e.g. 'Blueprint', 'StaticMesh', 'Material', 'Texture2D').",
                    },
                    "max_results": {
                        "type": "number",
                        "description": "Maximum results to return (default 100).",
                    },
                },
            },
        ),
    ]


def get_handlers() -> dict:
    return {
        "execute_console_command": handle_execute_console_command,
        "save_asset": handle_save_asset,
        "save_current_level": handle_save_current_level,
        "undo": handle_undo,
        "redo": handle_redo,
        "create_blueprint": handle_create_blueprint,
        "duplicate_asset": handle_duplicate_asset,
        "delete_asset": handle_delete_asset,
        "rename_asset": handle_rename_asset,
        "open_level": handle_open_level,
        "list_assets": handle_list_assets,
    }


async def handle_execute_console_command(args: dict) -> str:
    resp = await bridge.send_command("execute_console_command", {"command": args["command"]})
    if "error" in resp:
        return f"Error: {resp['error']}"
    return f"Executed: {resp.get('executed', args['command'])}"


async def handle_save_asset(args: dict) -> str:
    resp = await bridge.send_command("save_asset", {"asset_path": args["asset_path"]})
    if "error" in resp:
        return f"Error: {resp['error']}"
    return f"Saved: {resp.get('saved', args['asset_path'])}"


async def handle_save_current_level(args: dict) -> str:
    resp = await bridge.send_command("save_current_level", {})
    if "error" in resp:
        return f"Error: {resp['error']}"
    return f"Level '{resp.get('level', '')}' saved: {resp.get('saved', False)}"


async def handle_undo(args: dict) -> str:
    resp = await bridge.send_command("undo", {})
    if "error" in resp:
        return f"Error: {resp['error']}"
    return "Undo " + ("successful" if resp.get("undone") else "failed (nothing to undo)")


async def handle_redo(args: dict) -> str:
    resp = await bridge.send_command("redo", {})
    if "error" in resp:
        return f"Error: {resp['error']}"
    return "Redo " + ("successful" if resp.get("redone") else "failed (nothing to redo)")


async def handle_create_blueprint(args: dict) -> str:
    params = {"name": args["name"]}
    if "parent_class" in args:
        params["parent_class"] = args["parent_class"]
    if "path" in args:
        params["path"] = args["path"]

    resp = await bridge.send_command("create_blueprint", params)
    if "error" in resp:
        return f"Error: {resp['error']}"

    return (
        f"Created Blueprint '{resp.get('name', '')}'\n"
        f"  Path: {resp.get('path', '')}\n"
        f"  Parent: {resp.get('parent_class', '')}"
    )


async def handle_duplicate_asset(args: dict) -> str:
    resp = await bridge.send_command("duplicate_asset", {
        "source_path": args["source_path"],
        "dest_path": args["dest_path"],
        "dest_name": args["dest_name"],
    })
    if "error" in resp:
        return f"Error: {resp['error']}"
    return f"Duplicated {resp.get('source', '')} -> {resp.get('new_path', '')}"


async def handle_delete_asset(args: dict) -> str:
    resp = await bridge.send_command("delete_asset", {"asset_path": args["asset_path"]})
    if "error" in resp:
        return f"Error: {resp['error']}"
    return f"Deleted: {resp.get('deleted', '')} ({resp.get('count', 0)} objects)"


async def handle_rename_asset(args: dict) -> str:
    params = {
        "source_path": args["source_path"],
        "new_name": args["new_name"],
    }
    if "new_path" in args:
        params["new_path"] = args["new_path"]

    resp = await bridge.send_command("rename_asset", params)
    if "error" in resp:
        return f"Error: {resp['error']}"
    return f"Renamed {resp.get('old_path', '')} -> {resp.get('new_name', '')}"


async def handle_open_level(args: dict) -> str:
    resp = await bridge.send_command("open_level", {"level_path": args["level_path"]})
    if "error" in resp:
        return f"Error: {resp['error']}"
    return f"Opened level: {resp.get('opened', '')}"


async def handle_list_assets(args: dict) -> str:
    params = {}
    for key in ["path_filter", "class_filter", "max_results"]:
        if key in args:
            params[key] = args[key]

    resp = await bridge.send_command("list_assets", params)
    if "error" in resp:
        return f"Error: {resp['error']}"

    assets = resp.get("assets", [])
    total = resp.get("total", 0)
    if not assets:
        return "No assets found matching the filter."

    lines = [f"Found {total} assets (showing {len(assets)}):\n"]
    for a in assets:
        lines.append(f"  - {a['name']} ({a['class']})")
        lines.append(f"    {a['path']}")
    return "\n".join(lines)

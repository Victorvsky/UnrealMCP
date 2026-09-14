# Copyright (c) 2026 victorvksy. All rights reserved.

"""Blueprint macro tools — list, create, and read custom blueprint macros."""

from mcp.types import Tool

from unreal_mcp.bridge import bridge


def get_tools() -> list[Tool]:
    return [
        Tool(
            name="list_blueprint_macros",
            description=(
                "List all custom macros defined in a blueprint. "
                "Returns name, node count, inputs, and outputs for each macro."
            ),
            inputSchema={
                "type": "object",
                "properties": {
                    "blueprint": {
                        "type": "string",
                        "description": "Blueprint name or path.",
                    },
                },
                "required": ["blueprint"],
            },
        ),
        Tool(
            name="create_blueprint_macro",
            description=(
                "Create a new custom macro in a blueprint. "
                "Macros are reusable graph snippets with typed inputs/outputs. "
                "After creation, use add_blueprint_node with the macro's graph to add logic. "
                "Supported input/output types: exec, bool, int, float, string, Vector, Rotator, Transform, or any UClass name."
            ),
            inputSchema={
                "type": "object",
                "properties": {
                    "blueprint": {
                        "type": "string",
                        "description": "Blueprint name or path.",
                    },
                    "macro_name": {
                        "type": "string",
                        "description": "Name for the new macro.",
                    },
                    "inputs": {
                        "type": "array",
                        "description": "Input parameters. Each: {name, type}. 'exec' type adds an execution pin.",
                        "items": {
                            "type": "object",
                            "properties": {
                                "name": {"type": "string"},
                                "type": {"type": "string"},
                            },
                            "required": ["name", "type"],
                        },
                    },
                    "outputs": {
                        "type": "array",
                        "description": "Output parameters. Each: {name, type}. 'exec' type adds an execution pin.",
                        "items": {
                            "type": "object",
                            "properties": {
                                "name": {"type": "string"},
                                "type": {"type": "string"},
                            },
                            "required": ["name", "type"],
                        },
                    },
                },
                "required": ["blueprint", "macro_name"],
            },
        ),
        Tool(
            name="read_blueprint_macro_graph",
            description=(
                "Read the full graph of a custom blueprint macro: all nodes, pins, "
                "connections, and positions. Similar to read_blueprint_graph but for macros."
            ),
            inputSchema={
                "type": "object",
                "properties": {
                    "blueprint": {
                        "type": "string",
                        "description": "Blueprint name or path.",
                    },
                    "macro_name": {
                        "type": "string",
                        "description": "Name of the macro to read.",
                    },
                },
                "required": ["blueprint", "macro_name"],
            },
        ),
    ]


def get_handlers() -> dict:
    return {
        "list_blueprint_macros": handle_list_bp_macros,
        "create_blueprint_macro": handle_create_bp_macro,
        "read_blueprint_macro_graph": handle_read_bp_macro_graph,
    }


async def handle_list_bp_macros(args: dict) -> str:
    resp = await bridge.send_command("list_blueprint_macros", {
        "blueprint": args["blueprint"],
    })
    if "error" in resp:
        return f"Error: {resp['error']}"

    macros = resp.get("macros", [])
    if not macros:
        return f"No macros in {resp.get('blueprint', '')}."

    lines = [f"Blueprint: {resp.get('blueprint', '')}", f"Macros ({resp['count']}):\n"]
    for m in macros:
        inputs = [f"{p['name']}:{p['type']}" for p in m.get("inputs", [])]
        outputs = [f"{p['name']}:{p['type']}" for p in m.get("outputs", [])]
        in_str = ", ".join(inputs) if inputs else "none"
        out_str = ", ".join(outputs) if outputs else "none"
        lines.append(f"  - {m['name']} ({m.get('node_count', 0)} nodes)")
        lines.append(f"    In: ({in_str})  Out: ({out_str})")
    return "\n".join(lines)


async def handle_create_bp_macro(args: dict) -> str:
    params = {
        "blueprint": args["blueprint"],
        "macro_name": args["macro_name"],
    }
    if "inputs" in args:
        params["inputs"] = args["inputs"]
    if "outputs" in args:
        params["outputs"] = args["outputs"]

    resp = await bridge.send_command("create_blueprint_macro", params)
    if "error" in resp:
        return f"Error: {resp['error']}"

    lines = [
        f"Created macro '{resp.get('macro_name', '')}' in {resp.get('blueprint', '')}",
        f"Graph: {resp.get('graph_name', '')}",
    ]
    if "entry_node_id" in resp:
        lines.append(f"Entry node: {resp['entry_node_id']}")
    if "exit_node_id" in resp:
        lines.append(f"Exit node: {resp['exit_node_id']}")
    return "\n".join(lines)


async def handle_read_bp_macro_graph(args: dict) -> str:
    resp = await bridge.send_command("read_blueprint_macro_graph", {
        "blueprint": args["blueprint"],
        "macro_name": args["macro_name"],
    })
    if "error" in resp:
        return f"Error: {resp['error']}"

    lines = [
        f"Macro: {resp.get('macro_name', '')} in {resp.get('blueprint', '')}",
        f"Nodes ({resp.get('node_count', 0)}):\n",
    ]

    for node in resp.get("nodes", []):
        title = node.get("title", "?")
        nclass = node.get("class", "?")
        nid = node.get("id", "?")
        lines.append(f"  [{nid[:8]}] {title} ({nclass}) @ ({node.get('x', 0)}, {node.get('y', 0)})")

        for pin in node.get("pins", []):
            direction = pin.get("direction", "?")
            ptype = pin.get("type", "?")
            default = pin.get("default", "")
            conns = pin.get("connections", [])
            conn_str = ""
            if conns:
                conn_str = " -> " + ", ".join(
                    f"{c['node_id'][:8]}:{c['pin_name']}" for c in conns
                )
            default_str = f" = {default}" if default else ""
            lines.append(f"    {direction} {pin['name']} ({ptype}){default_str}{conn_str}")

    return "\n".join(lines)

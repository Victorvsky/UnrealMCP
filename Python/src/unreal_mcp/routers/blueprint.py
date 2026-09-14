# Copyright (c) 2026 victorvksy. All rights reserved.

"""Blueprint tools — list, read, edit, and compile Blueprints in the UE5 editor."""

from mcp.types import Tool

from unreal_mcp.bridge import bridge


def get_tools() -> list[Tool]:
    return [
        Tool(
            name="list_blueprints",
            description=(
                "List all Blueprint assets in the project. "
                "Returns name, path, and parent class for each Blueprint. "
                "Use path_filter to narrow results (e.g. '/Game/Blueprints/' or 'BP_NPC')."
            ),
            inputSchema={
                "type": "object",
                "properties": {
                    "path_filter": {
                        "type": "string",
                        "description": "Substring to filter Blueprint paths (e.g. 'Blueprints', 'BP_NPC').",
                    },
                },
            },
        ),
        Tool(
            name="read_blueprint_graph",
            description=(
                "Read the full graph structure of a Blueprint: nodes, pins, connections, and variables. "
                "Essential for understanding existing Blueprint logic before making changes. "
                "Returns node IDs (GUIDs) needed for connect_blueprint_pins."
            ),
            inputSchema={
                "type": "object",
                "properties": {
                    "blueprint": {
                        "type": "string",
                        "description": "Blueprint name (e.g. 'BP_MyActor') or full path ('/Game/Blueprints/BP_MyActor').",
                    },
                    "graph_name": {
                        "type": "string",
                        "description": "Optional specific graph name to read. Reads all graphs if omitted.",
                    },
                },
                "required": ["blueprint"],
            },
        ),
        Tool(
            name="add_blueprint_node",
            description=(
                "Add a new node to a Blueprint graph. Supported node types:\n"
                "- CallFunction: Call any UE5 function. Requires function_name, optionally target_class.\n"
                "- Event: Add an event node (built-in like ReceiveBeginPlay, or custom).\n"
                "- VariableGet / VariableSet: Get/set a Blueprint variable. Requires variable_name.\n"
                "- Branch (or IfThenElse): Conditional branch node.\n"
                "- Sequence: Execution sequence node (multiple exec outputs).\n"
                "- MakeStruct / BreakStruct: Construct or decompose structs. Requires struct_name (e.g. 'Vector', 'Rotator', 'Transform', 'LinearColor', 'Vector2D').\n"
                "- Cast: Dynamic cast to a class. Requires target_class.\n"
                "- GetArrayItem / MakeArray: Array operations.\n"
                "- SpawnActorFromClass: Spawn actor node.\n"
                "- Select: Select node (pick value based on index).\n"
                "- SwitchOnInt / SwitchOnString / SwitchOnName: Switch/case nodes.\n"
                "- CallDelegate: Broadcast/call an event dispatcher. Requires delegate_name.\n"
                "- BindDelegate / RemoveDelegate / ClearDelegate: Dispatcher binding nodes. Requires delegate_name.\n"
                "- CreateDelegate: Create a delegate binding.\n"
                "- ForEachLoop: For-each loop macro.\n"
                "- MacroInstance: Any Blueprint macro by name. Requires macro_name (e.g. 'ForEachLoop', 'WhileLoop', 'IsValid', 'FlipFlop', 'DoOnce', 'Gate').\n"
                "Returns the node ID and pin names for connecting."
            ),
            inputSchema={
                "type": "object",
                "properties": {
                    "blueprint": {
                        "type": "string",
                        "description": "Blueprint name or path.",
                    },
                    "node_type": {
                        "type": "string",
                        "enum": [
                            "CallFunction", "Event", "VariableGet", "VariableSet",
                            "Branch", "IfThenElse", "Sequence",
                            "MakeStruct", "BreakStruct",
                            "Cast", "GetArrayItem", "MakeArray",
                            "SpawnActorFromClass", "Select",
                            "SwitchOnInt", "SwitchOnString", "SwitchOnName",
                            "CallDelegate", "BindDelegate", "RemoveDelegate", "ClearDelegate", "CreateDelegate",
                            "ForEachLoop", "MacroInstance",
                        ],
                        "description": "Type of node to create.",
                    },
                    "x": {"type": "number", "description": "X position in the graph (default 0)."},
                    "y": {"type": "number", "description": "Y position in the graph (default 0)."},
                    "graph_name": {
                        "type": "string",
                        "description": "Target graph name. Uses the event graph if omitted.",
                    },
                    "function_name": {
                        "type": "string",
                        "description": "For CallFunction: the function name (e.g. 'PrintString', 'SetActorLocation').",
                    },
                    "target_class": {
                        "type": "string",
                        "description": "For CallFunction: class owning the function. For Cast: class to cast to.",
                    },
                    "event_name": {
                        "type": "string",
                        "description": "For Event: event name (e.g. 'ReceiveBeginPlay', 'MyCustomEvent').",
                    },
                    "variable_name": {
                        "type": "string",
                        "description": "For VariableGet/VariableSet: the variable name.",
                    },
                    "struct_name": {
                        "type": "string",
                        "description": "For MakeStruct/BreakStruct: struct name (e.g. 'Vector', 'Rotator', 'Transform').",
                    },
                    "delegate_name": {
                        "type": "string",
                        "description": "For delegate nodes: the event dispatcher name.",
                    },
                    "macro_name": {
                        "type": "string",
                        "description": "For MacroInstance: the macro name (e.g. 'ForEachLoop', 'WhileLoop', 'IsValid').",
                    },
                },
                "required": ["blueprint", "node_type"],
            },
        ),
        Tool(
            name="connect_blueprint_pins",
            description=(
                "Connect two pins in a Blueprint graph by node ID and pin name. "
                "Use read_blueprint_graph first to get node IDs and pin names. "
                "Automatically compiles the Blueprint after connecting."
            ),
            inputSchema={
                "type": "object",
                "properties": {
                    "blueprint": {
                        "type": "string",
                        "description": "Blueprint name or path.",
                    },
                    "source_node_id": {
                        "type": "string",
                        "description": "GUID of the source node (from read_blueprint_graph).",
                    },
                    "source_pin": {
                        "type": "string",
                        "description": "Name of the output pin on the source node.",
                    },
                    "target_node_id": {
                        "type": "string",
                        "description": "GUID of the target node.",
                    },
                    "target_pin": {
                        "type": "string",
                        "description": "Name of the input pin on the target node.",
                    },
                },
                "required": ["blueprint", "source_node_id", "source_pin", "target_node_id", "target_pin"],
            },
        ),
        Tool(
            name="disconnect_blueprint_pins",
            description=(
                "Disconnect pins in a Blueprint graph. Can disconnect a specific pin from all connections, "
                "or disconnect two specific pins from each other."
            ),
            inputSchema={
                "type": "object",
                "properties": {
                    "blueprint": {
                        "type": "string",
                        "description": "Blueprint name or path.",
                    },
                    "node_id": {
                        "type": "string",
                        "description": "GUID of the node whose pin to disconnect.",
                    },
                    "pin_name": {
                        "type": "string",
                        "description": "Name of the pin to disconnect.",
                    },
                    "target_node_id": {
                        "type": "string",
                        "description": "Optional: GUID of the specific target node to disconnect from.",
                    },
                    "target_pin": {
                        "type": "string",
                        "description": "Optional: Name of the specific target pin to disconnect from.",
                    },
                },
                "required": ["blueprint", "node_id", "pin_name"],
            },
        ),
        Tool(
            name="remove_blueprint_node",
            description=(
                "Remove a node from a Blueprint graph by its node ID. "
                "Breaks all pin connections before removing."
            ),
            inputSchema={
                "type": "object",
                "properties": {
                    "blueprint": {
                        "type": "string",
                        "description": "Blueprint name or path.",
                    },
                    "node_id": {
                        "type": "string",
                        "description": "GUID of the node to remove.",
                    },
                },
                "required": ["blueprint", "node_id"],
            },
        ),
        Tool(
            name="set_pin_default_value",
            description=(
                "Set the default value of an input pin on a Blueprint node. "
                "Use read_blueprint_graph to find the node ID and pin name."
            ),
            inputSchema={
                "type": "object",
                "properties": {
                    "blueprint": {
                        "type": "string",
                        "description": "Blueprint name or path.",
                    },
                    "node_id": {
                        "type": "string",
                        "description": "GUID of the node.",
                    },
                    "pin_name": {
                        "type": "string",
                        "description": "Name of the input pin.",
                    },
                    "value": {
                        "type": "string",
                        "description": "Default value as string (e.g. '42', 'true', 'Hello', '1.0,2.0,3.0' for vectors).",
                    },
                },
                "required": ["blueprint", "node_id", "pin_name", "value"],
            },
        ),
        Tool(
            name="compile_blueprint",
            description=(
                "Force-compile a Blueprint and return the result. "
                "Reports whether compilation succeeded and any error messages."
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
            name="add_blueprint_variable",
            description=(
                "Add a new variable to a Blueprint. Supported types: "
                "bool, int, float, string, text, name, Vector, Rotator, Transform, "
                "or any UE5 class name for object references."
            ),
            inputSchema={
                "type": "object",
                "properties": {
                    "blueprint": {
                        "type": "string",
                        "description": "Blueprint name or path.",
                    },
                    "variable_name": {
                        "type": "string",
                        "description": "Name for the new variable.",
                    },
                    "variable_type": {
                        "type": "string",
                        "description": "Type: bool, int, float, string, text, name, Vector, Rotator, Transform, or a UClass name.",
                    },
                    "is_array": {
                        "type": "boolean",
                        "description": "Whether this variable is an array (default false).",
                    },
                    "instance_editable": {
                        "type": "boolean",
                        "description": "Whether this variable is editable per-instance in the editor (default false).",
                    },
                    "default_value": {
                        "type": "string",
                        "description": "Default value as a string.",
                    },
                },
                "required": ["blueprint", "variable_name", "variable_type"],
            },
        ),
        Tool(
            name="remove_blueprint_variable",
            description="Remove a variable from a Blueprint by name.",
            inputSchema={
                "type": "object",
                "properties": {
                    "blueprint": {
                        "type": "string",
                        "description": "Blueprint name or path.",
                    },
                    "variable_name": {
                        "type": "string",
                        "description": "Name of the variable to remove.",
                    },
                },
                "required": ["blueprint", "variable_name"],
            },
        ),
        Tool(
            name="add_blueprint_function",
            description=(
                "Create a new function graph in a Blueprint. "
                "Returns the function entry and result node IDs for adding logic. "
                "Optionally specify input and output parameters."
            ),
            inputSchema={
                "type": "object",
                "properties": {
                    "blueprint": {
                        "type": "string",
                        "description": "Blueprint name or path.",
                    },
                    "function_name": {
                        "type": "string",
                        "description": "Name for the new function.",
                    },
                    "inputs": {
                        "type": "array",
                        "description": "Input parameters: [{name, type}]. Types: bool, int, float, string, Vector, Rotator, Transform, or class names.",
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
                        "description": "Output (return) parameters: [{name, type}].",
                        "items": {
                            "type": "object",
                            "properties": {
                                "name": {"type": "string"},
                                "type": {"type": "string"},
                            },
                            "required": ["name", "type"],
                        },
                    },
                    "is_pure": {
                        "type": "boolean",
                        "description": "Whether this is a pure function (no exec pins). Default false.",
                    },
                },
                "required": ["blueprint", "function_name"],
            },
        ),
        Tool(
            name="add_event_dispatcher",
            description=(
                "Add an event dispatcher (multicast delegate) to a Blueprint. "
                "Optionally define parameter signatures for the dispatcher. "
                "Once added, use add_blueprint_node with CallDelegate/BindDelegate/RemoveDelegate node types to work with it."
            ),
            inputSchema={
                "type": "object",
                "properties": {
                    "blueprint": {
                        "type": "string",
                        "description": "Blueprint name or path.",
                    },
                    "name": {
                        "type": "string",
                        "description": "Name for the event dispatcher.",
                    },
                    "params": {
                        "type": "array",
                        "description": "Optional parameter definitions: [{name, type}]. Types: bool, int, float, string, Vector, Rotator, Transform, or class names.",
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
                "required": ["blueprint", "name"],
            },
        ),
        Tool(
            name="add_blueprint_component",
            description=(
                "Add a component to a Blueprint's component hierarchy (Simple Construction Script). "
                "The component will appear in the Blueprint's Components panel."
            ),
            inputSchema={
                "type": "object",
                "properties": {
                    "blueprint": {
                        "type": "string",
                        "description": "Blueprint name or path.",
                    },
                    "component_class": {
                        "type": "string",
                        "description": "Component class name (e.g. 'StaticMeshComponent', 'PointLightComponent', 'BoxCollisionComponent', 'AudioComponent').",
                    },
                    "component_name": {
                        "type": "string",
                        "description": "Optional name for the component.",
                    },
                },
                "required": ["blueprint", "component_class"],
            },
        ),
    ]


def get_handlers() -> dict:
    return {
        "list_blueprints": handle_list_blueprints,
        "read_blueprint_graph": handle_read_blueprint_graph,
        "add_blueprint_node": handle_add_blueprint_node,
        "connect_blueprint_pins": handle_connect_blueprint_pins,
        "disconnect_blueprint_pins": handle_disconnect_blueprint_pins,
        "remove_blueprint_node": handle_remove_blueprint_node,
        "set_pin_default_value": handle_set_pin_default_value,
        "compile_blueprint": handle_compile_blueprint,
        "add_blueprint_variable": handle_add_blueprint_variable,
        "remove_blueprint_variable": handle_remove_blueprint_variable,
        "add_blueprint_function": handle_add_blueprint_function,
        "add_event_dispatcher": handle_add_event_dispatcher,
        "add_blueprint_component": handle_add_blueprint_component,
    }


async def handle_list_blueprints(args: dict) -> str:
    resp = await bridge.send_command("list_blueprints", {
        k: v for k, v in {"path_filter": args.get("path_filter")}.items() if v
    })

    if "error" in resp:
        return f"Error: {resp['error']}"

    blueprints = resp.get("blueprints", [])
    if not blueprints:
        return "No Blueprints found matching the filter."

    lines = [f"Found {resp['count']} Blueprints:\n"]
    for bp in blueprints:
        parent = bp.get("parent_class", "Unknown")
        # Shorten parent class path for readability
        if "." in parent:
            parent = parent.rsplit(".", 1)[-1].strip("'")
        lines.append(f"  - {bp['name']}")
        lines.append(f"    Path: {bp['path']}")
        lines.append(f"    Parent: {parent}")
    return "\n".join(lines)


async def handle_read_blueprint_graph(args: dict) -> str:
    params = {"blueprint": args["blueprint"]}
    if "graph_name" in args:
        params["graph_name"] = args["graph_name"]

    resp = await bridge.send_command("read_bp_graph", params)

    if "error" in resp:
        return f"Error: {resp['error']}"

    lines = [f"Blueprint: {resp.get('blueprint', args['blueprint'])}\n"]

    # Variables
    variables = resp.get("variables", [])
    if variables:
        lines.append("Variables:")
        for v in variables:
            sub = f" ({v['sub_type']})" if v.get("sub_type") else ""
            editable = " [Instance Editable]" if v.get("is_instance_editable") else ""
            lines.append(f"  - {v['name']}: {v['type']}{sub}{editable}")
        lines.append("")

    # Graphs
    graphs = resp.get("graphs", [])
    for graph in graphs:
        graph_type = f" [{graph['type']}]" if graph.get("type") else ""
        lines.append(f"Graph: {graph['name']}{graph_type}")
        nodes = graph.get("nodes", [])
        if not nodes:
            lines.append("  (empty graph)")
            continue

        for node in nodes:
            lines.append(f"  Node: {node['title']}")
            lines.append(f"    ID: {node['id']}")
            lines.append(f"    Class: {node['class']}")
            lines.append(f"    Position: ({node['x']}, {node['y']})")

            pins = node.get("pins", [])
            if pins:
                inputs = [p for p in pins if p["direction"] == "input"]
                outputs = [p for p in pins if p["direction"] == "output"]

                if inputs:
                    lines.append("    Inputs:")
                    for p in inputs:
                        default = f" = {p['default_value']}" if p.get("default_value") else ""
                        conns = ""
                        if p.get("connections"):
                            conn_strs = [f"{c['node_id'][:8]}...:{c['pin_name']}" for c in p["connections"]]
                            conns = f" -> [{', '.join(conn_strs)}]"
                        lines.append(f"      {p['name']} ({p['type']}){default}{conns}")

                if outputs:
                    lines.append("    Outputs:")
                    for p in outputs:
                        conns = ""
                        if p.get("connections"):
                            conn_strs = [f"{c['node_id'][:8]}...:{c['pin_name']}" for c in p["connections"]]
                            conns = f" -> [{', '.join(conn_strs)}]"
                        lines.append(f"      {p['name']} ({p['type']}){conns}")
            lines.append("")

    return "\n".join(lines)


async def handle_add_blueprint_node(args: dict) -> str:
    params = {
        "blueprint": args["blueprint"],
        "node_type": args["node_type"],
        "x": args.get("x", 0),
        "y": args.get("y", 0),
    }

    for key in [
        "graph_name", "function_name", "target_class", "event_name",
        "variable_name", "struct_name", "delegate_name", "macro_name",
    ]:
        if key in args:
            params[key] = args[key]

    resp = await bridge.send_command("add_bp_node", params)

    if "error" in resp:
        return f"Error adding node: {resp['error']}"

    lines = [
        f"Added node: {resp['title']}",
        f"  Node ID: {resp['node_id']}",
        f"  Class: {resp['node_class']}",
    ]

    pins = resp.get("pins", [])
    if pins:
        inputs = [p for p in pins if p["direction"] == "input"]
        outputs = [p for p in pins if p["direction"] == "output"]

        if inputs:
            lines.append("  Input pins:")
            for p in inputs:
                lines.append(f"    - {p['name']} ({p['type']})")
        if outputs:
            lines.append("  Output pins:")
            for p in outputs:
                lines.append(f"    - {p['name']} ({p['type']})")

    return "\n".join(lines)


async def handle_connect_blueprint_pins(args: dict) -> str:
    resp = await bridge.send_command("connect_pins", {
        "blueprint": args["blueprint"],
        "source_node_id": args["source_node_id"],
        "source_pin": args["source_pin"],
        "target_node_id": args["target_node_id"],
        "target_pin": args["target_pin"],
    })

    if "error" in resp:
        return f"Error connecting pins: {resp['error']}"

    compiled = " (Blueprint compiled)" if resp.get("compiled") else ""
    return f"Connected {resp['source']} -> {resp['target']}{compiled}"


async def handle_disconnect_blueprint_pins(args: dict) -> str:
    params = {
        "blueprint": args["blueprint"],
        "node_id": args["node_id"],
        "pin_name": args["pin_name"],
    }
    if "target_node_id" in args:
        params["target_node_id"] = args["target_node_id"]
    if "target_pin" in args:
        params["target_pin"] = args["target_pin"]

    resp = await bridge.send_command("disconnect_pins", params)

    if "error" in resp:
        return f"Error disconnecting pins: {resp['error']}"

    return f"Disconnected: {resp.get('disconnected_pin', 'pin')}"


async def handle_remove_blueprint_node(args: dict) -> str:
    resp = await bridge.send_command("remove_bp_node", {
        "blueprint": args["blueprint"],
        "node_id": args["node_id"],
    })

    if "error" in resp:
        return f"Error removing node: {resp['error']}"

    return f"Removed node: {resp.get('removed_node', '')} (ID: {resp.get('node_id', '')})"


async def handle_set_pin_default_value(args: dict) -> str:
    resp = await bridge.send_command("set_pin_default", {
        "blueprint": args["blueprint"],
        "node_id": args["node_id"],
        "pin_name": args["pin_name"],
        "value": args["value"],
    })

    if "error" in resp:
        return f"Error setting pin default: {resp['error']}"

    return f"Set {resp.get('pin_name', '')} on node {resp.get('node_id', '')[:8]}... = {resp.get('new_value', '')}"


async def handle_compile_blueprint(args: dict) -> str:
    resp = await bridge.send_command("compile_bp", {"blueprint": args["blueprint"]})

    if "error" in resp:
        return f"Error: {resp['error']}"

    lines = [f"Blueprint: {resp.get('blueprint', args['blueprint'])}"]
    lines.append(f"Compilation: {'FAILED' if resp.get('has_errors') else 'SUCCESS'}")

    messages = resp.get("messages", [])
    if messages:
        lines.append("Messages:")
        for msg in messages:
            lines.append(f"  - {msg}")

    return "\n".join(lines)


async def handle_add_blueprint_variable(args: dict) -> str:
    params = {
        "blueprint": args["blueprint"],
        "variable_name": args["variable_name"],
        "variable_type": args["variable_type"],
    }

    for key in ["is_array", "instance_editable", "default_value"]:
        if key in args:
            params[key] = args[key]

    resp = await bridge.send_command("add_bp_variable", params)

    if "error" in resp:
        return f"Error adding variable: {resp['error']}"

    return (
        f"Added variable '{resp['variable_name']}' of type '{resp['variable_type']}' "
        f"to {resp['blueprint']}"
    )


async def handle_remove_blueprint_variable(args: dict) -> str:
    resp = await bridge.send_command("remove_bp_variable", {
        "blueprint": args["blueprint"],
        "variable_name": args["variable_name"],
    })

    if "error" in resp:
        return f"Error removing variable: {resp['error']}"

    return f"Removed variable '{resp.get('removed_variable', '')}' from {resp.get('blueprint', '')}"


async def handle_add_blueprint_function(args: dict) -> str:
    params = {
        "blueprint": args["blueprint"],
        "function_name": args["function_name"],
    }

    for key in ["inputs", "outputs", "is_pure"]:
        if key in args:
            params[key] = args[key]

    resp = await bridge.send_command("add_bp_function", params)

    if "error" in resp:
        return f"Error adding function: {resp['error']}"

    lines = [
        f"Created function '{resp.get('function_name', '')}' in {resp.get('blueprint', '')}",
        f"  Graph: {resp.get('graph_name', '')}",
    ]
    if resp.get("entry_node_id"):
        lines.append(f"  Entry node ID: {resp['entry_node_id']}")
    if resp.get("result_node_id"):
        lines.append(f"  Result node ID: {resp['result_node_id']}")

    return "\n".join(lines)


async def handle_add_event_dispatcher(args: dict) -> str:
    params = {
        "blueprint": args["blueprint"],
        "name": args["name"],
    }
    if "params" in args:
        params["params"] = args["params"]

    resp = await bridge.send_command("add_event_dispatcher", params)

    if "error" in resp:
        return f"Error adding event dispatcher: {resp['error']}"

    result = f"Added event dispatcher '{resp.get('dispatcher_name', '')}' to {resp.get('blueprint', '')}"
    param_names = resp.get("parameters", [])
    if param_names:
        result += f"\n  Parameters: {', '.join(param_names)}"
    return result


async def handle_add_blueprint_component(args: dict) -> str:
    params = {
        "blueprint": args["blueprint"],
        "component_class": args["component_class"],
    }
    if "component_name" in args:
        params["component_name"] = args["component_name"]

    resp = await bridge.send_command("add_bp_component", params)

    if "error" in resp:
        return f"Error adding component: {resp['error']}"

    return (
        f"Added component '{resp.get('component_name', '')}' ({resp.get('component_class', '')}) "
        f"to {resp.get('blueprint', '')}"
    )

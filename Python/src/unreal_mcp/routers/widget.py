# Copyright (c) 2026 victorvksy. All rights reserved.

"""Widget/UMG tools — read, edit, and manage Widget Blueprint hierarchies."""

from mcp.types import Tool

from unreal_mcp.bridge import bridge


def get_tools() -> list[Tool]:
    return [
        Tool(
            name="read_widget_tree",
            description=(
                "Read the full widget hierarchy of a Widget Blueprint. "
                "Returns the tree structure, widget names, classes, and all widgets."
            ),
            inputSchema={
                "type": "object",
                "properties": {
                    "widget_blueprint": {
                        "type": "string",
                        "description": "Widget Blueprint name (e.g. 'WBP_HUD') or full path.",
                    },
                },
                "required": ["widget_blueprint"],
            },
        ),
        Tool(
            name="list_widget_children",
            description=(
                "List the direct children of a panel widget in a Widget Blueprint. "
                "If no parent is specified, lists children of the root widget."
            ),
            inputSchema={
                "type": "object",
                "properties": {
                    "widget_blueprint": {
                        "type": "string",
                        "description": "Widget Blueprint name or path.",
                    },
                    "parent_name": {
                        "type": "string",
                        "description": "Name of the parent panel widget. Uses root if omitted.",
                    },
                },
                "required": ["widget_blueprint"],
            },
        ),
        Tool(
            name="add_widget_child",
            description=(
                "Add a new widget to a Widget Blueprint hierarchy. "
                "Supported widget classes include: TextBlock, Image, Button, "
                "CanvasPanel, VerticalBox, HorizontalBox, ProgressBar, Border, "
                "Overlay, SizeBox, and any other UWidget subclass."
            ),
            inputSchema={
                "type": "object",
                "properties": {
                    "widget_blueprint": {
                        "type": "string",
                        "description": "Widget Blueprint name or path.",
                    },
                    "widget_class": {
                        "type": "string",
                        "description": "Widget class to add (e.g. 'TextBlock', 'Image', 'Button', 'VerticalBox').",
                    },
                    "parent_name": {
                        "type": "string",
                        "description": "Name of the parent panel to add to. Uses root if omitted.",
                    },
                    "name": {
                        "type": "string",
                        "description": "Optional name for the new widget.",
                    },
                },
                "required": ["widget_blueprint", "widget_class"],
            },
        ),
        Tool(
            name="remove_widget_child",
            description="Remove a widget from a Widget Blueprint hierarchy by name.",
            inputSchema={
                "type": "object",
                "properties": {
                    "widget_blueprint": {
                        "type": "string",
                        "description": "Widget Blueprint name or path.",
                    },
                    "widget_name": {
                        "type": "string",
                        "description": "Name of the widget to remove.",
                    },
                },
                "required": ["widget_blueprint", "widget_name"],
            },
        ),
        Tool(
            name="set_widget_property",
            description=(
                "Set a property on a widget in a Widget Blueprint using reflection. "
                "Works with any property that has a text export format."
            ),
            inputSchema={
                "type": "object",
                "properties": {
                    "widget_blueprint": {
                        "type": "string",
                        "description": "Widget Blueprint name or path.",
                    },
                    "widget_name": {
                        "type": "string",
                        "description": "Name of the widget to modify.",
                    },
                    "property_name": {
                        "type": "string",
                        "description": "Property name (e.g. 'Visibility', 'RenderOpacity', 'ColorAndOpacity').",
                    },
                    "value": {
                        "type": "string",
                        "description": "Property value as a string in UE5 text export format.",
                    },
                },
                "required": ["widget_blueprint", "widget_name", "property_name", "value"],
            },
        ),
        Tool(
            name="set_widget_slot",
            description=(
                "Set slot properties on a widget (how it's laid out within its parent panel). "
                "Properties vary by parent type:\n"
                "- CanvasPanel: anchors, offsets, position, size, alignment, auto_size, z_order.\n"
                "- VerticalBox/HorizontalBox: padding, size_rule (Auto/Fill), h_align, v_align.\n"
                "- Overlay: padding, h_align, v_align.\n"
                "- Other slots: use property_name + value for generic reflection access."
            ),
            inputSchema={
                "type": "object",
                "properties": {
                    "widget_blueprint": {
                        "type": "string",
                        "description": "Widget Blueprint name or path.",
                    },
                    "widget_name": {
                        "type": "string",
                        "description": "Name of the widget whose slot to modify.",
                    },
                    "anchors": {
                        "type": "object",
                        "description": "Canvas slot anchors: {min_x, min_y, max_x, max_y} (0-1). E.g. {min_x:0, min_y:0, max_x:1, max_y:1} for stretch.",
                        "properties": {
                            "min_x": {"type": "number"}, "min_y": {"type": "number"},
                            "max_x": {"type": "number"}, "max_y": {"type": "number"},
                        },
                    },
                    "offsets": {
                        "type": "object",
                        "description": "Canvas slot offsets: {left, top, right, bottom}.",
                        "properties": {
                            "left": {"type": "number"}, "top": {"type": "number"},
                            "right": {"type": "number"}, "bottom": {"type": "number"},
                        },
                    },
                    "position": {
                        "type": "object",
                        "description": "Canvas slot position: {x, y}.",
                        "properties": {"x": {"type": "number"}, "y": {"type": "number"}},
                    },
                    "size": {
                        "type": "object",
                        "description": "Canvas slot size: {width, height}.",
                        "properties": {"width": {"type": "number"}, "height": {"type": "number"}},
                    },
                    "alignment": {
                        "type": "object",
                        "description": "Canvas slot alignment pivot: {x, y} (0-1).",
                        "properties": {"x": {"type": "number"}, "y": {"type": "number"}},
                    },
                    "auto_size": {
                        "type": "boolean",
                        "description": "Canvas slot: auto-size to content.",
                    },
                    "z_order": {
                        "type": "number",
                        "description": "Canvas slot: Z-order (draw order).",
                    },
                    "padding": {
                        "type": "object",
                        "description": "Slot padding: {left, top, right, bottom} or {all} for uniform.",
                        "properties": {
                            "left": {"type": "number"}, "top": {"type": "number"},
                            "right": {"type": "number"}, "bottom": {"type": "number"},
                            "all": {"type": "number"},
                        },
                    },
                    "size_rule": {
                        "type": "string",
                        "enum": ["Auto", "Fill"],
                        "description": "Box slot size rule.",
                    },
                    "fill_value": {
                        "type": "number",
                        "description": "Fill weight when size_rule is Fill (default 1.0).",
                    },
                    "h_align": {
                        "type": "string",
                        "enum": ["Fill", "Left", "Center", "Right"],
                        "description": "Horizontal alignment.",
                    },
                    "v_align": {
                        "type": "string",
                        "enum": ["Fill", "Top", "Center", "Bottom"],
                        "description": "Vertical alignment.",
                    },
                    "property_name": {
                        "type": "string",
                        "description": "For unsupported slot types: property name for generic access.",
                    },
                    "value": {
                        "type": "string",
                        "description": "For unsupported slot types: property value string.",
                    },
                },
                "required": ["widget_blueprint", "widget_name"],
            },
        ),
    ]


def get_handlers() -> dict:
    return {
        "read_widget_tree": handle_read_widget_tree,
        "list_widget_children": handle_list_widget_children,
        "add_widget_child": handle_add_widget_child,
        "remove_widget_child": handle_remove_widget_child,
        "set_widget_property": handle_set_widget_property,
        "set_widget_slot": handle_set_widget_slot,
    }


def _format_widget_tree(widget: dict, indent: int = 0) -> list[str]:
    """Recursively format a widget tree for display."""
    prefix = "  " * indent
    lines = [f"{prefix}- {widget['name']} ({widget['class']})"]

    if widget.get("text"):
        lines.append(f"{prefix}  text: \"{widget['text']}\"")

    for child in widget.get("children", []):
        lines.extend(_format_widget_tree(child, indent + 1))

    return lines


async def handle_read_widget_tree(args: dict) -> str:
    resp = await bridge.send_command("read_widget_tree", {
        "widget_blueprint": args["widget_blueprint"],
    })
    if "error" in resp:
        return f"Error: {resp['error']}"

    lines = [f"Widget Blueprint: {resp.get('widget_blueprint', '')}\n"]

    tree = resp.get("tree")
    if tree:
        lines.append("Widget Tree:")
        lines.extend(_format_widget_tree(tree))
    else:
        lines.append("(empty widget tree)")

    lines.append(f"\nTotal widgets: {resp.get('widget_count', 0)}")
    return "\n".join(lines)


async def handle_list_widget_children(args: dict) -> str:
    params = {"widget_blueprint": args["widget_blueprint"]}
    if "parent_name" in args:
        params["parent_name"] = args["parent_name"]

    resp = await bridge.send_command("list_widget_children", params)
    if "error" in resp:
        return f"Error: {resp['error']}"

    children = resp.get("children", [])
    if not children:
        return f"No children in '{resp.get('parent', 'root')}'."

    lines = [f"Children of '{resp.get('parent', 'root')}' ({resp.get('count', 0)}):\n"]
    for c in children:
        lines.append(f"  [{c.get('index', '?')}] {c['name']} ({c['class']})")
    return "\n".join(lines)


async def handle_add_widget_child(args: dict) -> str:
    params = {
        "widget_blueprint": args["widget_blueprint"],
        "widget_class": args["widget_class"],
    }
    if "parent_name" in args:
        params["parent_name"] = args["parent_name"]
    if "name" in args:
        params["name"] = args["name"]

    resp = await bridge.send_command("add_widget_child", params)
    if "error" in resp:
        return f"Error: {resp['error']}"

    root_msg = " (set as root)" if resp.get("set_as_root") else f" in '{resp.get('parent', 'root')}'"
    return f"Added widget '{resp.get('name', '')}' ({resp.get('class', '')}){root_msg}"


async def handle_remove_widget_child(args: dict) -> str:
    resp = await bridge.send_command("remove_widget_child", {
        "widget_blueprint": args["widget_blueprint"],
        "widget_name": args["widget_name"],
    })
    if "error" in resp:
        return f"Error: {resp['error']}"

    return f"Removed widget: {resp.get('removed', '')}"


async def handle_set_widget_property(args: dict) -> str:
    resp = await bridge.send_command("set_widget_property", {
        "widget_blueprint": args["widget_blueprint"],
        "widget_name": args["widget_name"],
        "property_name": args["property_name"],
        "value": args["value"],
    })
    if "error" in resp:
        return f"Error: {resp['error']}"

    return f"Set {resp.get('widget', '')}.{resp.get('property', '')} = {resp.get('value', '')}"


async def handle_set_widget_slot(args: dict) -> str:
    params = {
        "widget_blueprint": args["widget_blueprint"],
        "widget_name": args["widget_name"],
    }
    # Pass through all optional slot properties
    for key in [
        "anchors", "offsets", "position", "size", "alignment",
        "auto_size", "z_order", "padding", "size_rule", "fill_value",
        "h_align", "v_align", "property_name", "value",
    ]:
        if key in args:
            params[key] = args[key]

    resp = await bridge.send_command("set_widget_slot", params)
    if "error" in resp:
        return f"Error: {resp['error']}"

    slot_type = resp.get("slot_type", "Unknown")
    return f"Updated slot ({slot_type}) for widget '{resp.get('widget', '')}'"

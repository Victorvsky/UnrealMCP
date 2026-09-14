# Copyright (c) 2026 victorvksy. All rights reserved.

"""Component tools — list, add, and remove components on actors in the level."""

from mcp.types import Tool

from unreal_mcp.bridge import bridge


def get_tools() -> list[Tool]:
    return [
        Tool(
            name="list_components",
            description=(
                "List all components on an actor in the current level. "
                "Returns each component's name, class, position (if scene component), and status."
            ),
            inputSchema={
                "type": "object",
                "properties": {
                    "actor_name": {
                        "type": "string",
                        "description": "Name of the actor to inspect.",
                    },
                },
                "required": ["actor_name"],
            },
        ),
        Tool(
            name="add_actor_component",
            description=(
                "Add a new component to an actor in the current level at runtime. "
                "For adding components to Blueprints, use add_blueprint_component instead."
            ),
            inputSchema={
                "type": "object",
                "properties": {
                    "actor_name": {
                        "type": "string",
                        "description": "Name of the actor to add the component to.",
                    },
                    "component_class": {
                        "type": "string",
                        "description": "Component class (e.g. 'PointLightComponent', 'StaticMeshComponent', 'AudioComponent').",
                    },
                    "component_name": {
                        "type": "string",
                        "description": "Optional name for the new component.",
                    },
                },
                "required": ["actor_name", "component_class"],
            },
        ),
        Tool(
            name="remove_actor_component",
            description="Remove a component from an actor in the current level.",
            inputSchema={
                "type": "object",
                "properties": {
                    "actor_name": {
                        "type": "string",
                        "description": "Name of the actor.",
                    },
                    "component_name": {
                        "type": "string",
                        "description": "Name of the component to remove.",
                    },
                },
                "required": ["actor_name", "component_name"],
            },
        ),
    ]


def get_handlers() -> dict:
    return {
        "list_components": handle_list_components,
        "add_actor_component": handle_add_actor_component,
        "remove_actor_component": handle_remove_actor_component,
    }


async def handle_list_components(args: dict) -> str:
    resp = await bridge.send_command("list_components", {"actor_name": args["actor_name"]})
    if "error" in resp:
        return f"Error: {resp['error']}"

    components = resp.get("components", [])
    if not components:
        return f"No components found on '{resp.get('actor', args['actor_name'])}'."

    lines = [f"Components on '{resp.get('actor', '')}' ({resp.get('count', 0)}):\n"]
    for c in components:
        root = " [ROOT]" if c.get("is_root") else ""
        active = "" if c.get("is_active") else " [INACTIVE]"
        pos = ""
        if "x" in c:
            pos = f" at ({c['x']:.1f}, {c['y']:.1f}, {c['z']:.1f})"
        lines.append(f"  - {c['name']} ({c['class']}){root}{active}{pos}")
    return "\n".join(lines)


async def handle_add_actor_component(args: dict) -> str:
    params = {
        "actor_name": args["actor_name"],
        "component_class": args["component_class"],
    }
    if "component_name" in args:
        params["component_name"] = args["component_name"]

    resp = await bridge.send_command("add_actor_component", params)
    if "error" in resp:
        return f"Error: {resp['error']}"

    return (
        f"Added '{resp.get('component_name', '')}' ({resp.get('component_class', '')}) "
        f"to actor '{resp.get('actor', '')}'"
    )


async def handle_remove_actor_component(args: dict) -> str:
    resp = await bridge.send_command("remove_actor_component", {
        "actor_name": args["actor_name"],
        "component_name": args["component_name"],
    })
    if "error" in resp:
        return f"Error: {resp['error']}"

    return f"Removed component '{resp.get('removed_component', '')}' from '{resp.get('actor', '')}'"

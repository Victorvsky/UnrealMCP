"""Unreal MCP Server — exposes Unreal Editor tools to AI agents via MCP protocol."""

import asyncio
import logging

from mcp.server import Server
from mcp.server.stdio import stdio_server
from mcp.types import Tool, TextContent

from unreal_mcp.bridge import bridge, BridgeError
from unreal_mcp.routers import actor, blueprint, level, landscape, material, playtest, editor, component, widget, asset, datatable, niagara, sequencer, widget_animation, blueprint_macro

logging.basicConfig(level=logging.INFO, format="%(asctime)s [%(name)s] %(message)s")
logger = logging.getLogger("unreal-mcp")

server = Server("unreal-mcp")

# Collect all tool definitions and handlers from routers
ALL_TOOLS: dict[str, Tool] = {}
ALL_HANDLERS: dict[str, any] = {}


def _register_router(module):
    """Register tools and handlers from a router module."""
    for tool in module.get_tools():
        ALL_TOOLS[tool.name] = tool
    ALL_HANDLERS.update(module.get_handlers())


_register_router(actor)
_register_router(blueprint)
_register_router(level)
_register_router(landscape)
_register_router(material)
_register_router(playtest)
_register_router(editor)
_register_router(component)
_register_router(widget)
_register_router(asset)
_register_router(datatable)
_register_router(niagara)
_register_router(sequencer)
_register_router(widget_animation)
_register_router(blueprint_macro)


@server.list_tools()
async def list_tools() -> list[Tool]:
    return list(ALL_TOOLS.values())


@server.call_tool()
async def call_tool(name: str, arguments: dict) -> list[TextContent]:
    handler = ALL_HANDLERS.get(name)
    if not handler:
        return [TextContent(type="text", text=f"Unknown tool: {name}")]

    try:
        result = await handler(arguments)
        return [TextContent(type="text", text=result)]
    except BridgeError as e:
        return [TextContent(type="text", text=f"UE5 Bridge Error: {e}")]
    except Exception as e:
        logger.exception(f"Error in tool {name}")
        return [TextContent(type="text", text=f"Error: {e}")]


async def main():
    logger.info("Starting Unreal MCP Server...")
    logger.info(f"Registered {len(ALL_TOOLS)} tools: {', '.join(ALL_TOOLS.keys())}")

    async with stdio_server() as (read_stream, write_stream):
        await server.run(read_stream, write_stream, server.create_initialization_options())


if __name__ == "__main__":
    asyncio.run(main())

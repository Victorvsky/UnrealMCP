# Copyright (c) 2026 victorvksy. All rights reserved.

"""The C++ command table and the Python tool routers must describe the same set of commands.

They are maintained by hand on both sides, and they had already drifted once
(``get_widget_property`` existed only in C++). Every new tool goes through this test.
"""

import re
from pathlib import Path

PLUGIN = Path(__file__).resolve().parents[2]
CPP = PLUGIN / "Source" / "UnrealMCP" / "Private" / "MCPTcpServer.cpp"
ROUTERS = PLUGIN / "Python" / "src" / "unreal_mcp" / "routers"

# Commands the plugin registers for its own use, with no MCP tool on purpose.
INTERNAL_COMMANDS: set[str] = set()


def cpp_commands() -> set[str]:
    return set(re.findall(r'RegisterHandler\(TEXT\("([a-z0-9_]+)"\)', CPP.read_text(encoding="utf-8")))


def python_commands() -> set[str]:
    out = set()
    for f in ROUTERS.glob("*.py"):
        out |= set(re.findall(r'send_command\(\s*"([a-z0-9_]+)"', f.read_text(encoding="utf-8")))
    return out


def test_cpp_commands_all_have_python_tools():
    missing = cpp_commands() - python_commands() - INTERNAL_COMMANDS
    assert not missing, f"C++ commands with no Python tool: {sorted(missing)}"


def test_python_tools_all_have_cpp_commands():
    missing = python_commands() - cpp_commands()
    assert not missing, f"Python tools sending unknown commands: {sorted(missing)}"


def test_every_tool_has_a_handler_and_a_schema():
    import sys

    sys.path.insert(0, str(PLUGIN / "Python" / "src"))
    from unreal_mcp import main

    tools, handlers = main.ALL_TOOLS, main.ALL_HANDLERS
    assert set(tools) == set(handlers), (
        f"tools without handlers: {sorted(set(tools) - set(handlers))}; "
        f"handlers without tools: {sorted(set(handlers) - set(tools))}"
    )
    for name, tool in tools.items():
        assert tool.inputSchema.get("type") == "object", f"{name}: inputSchema must be an object schema"
        assert tool.description, f"{name}: missing description"

# Copyright (c) 2026 victorvksy. All rights reserved.

"""Shared fixtures. Tests marked ``live`` need a running editor with the plugin loaded; they
are skipped automatically when ``Saved/UnrealMCP/port.txt`` cannot be found."""

import asyncio
import os
import sys
from pathlib import Path

import pytest

PLUGIN = Path(__file__).resolve().parents[2]
sys.path.insert(0, str(PLUGIN / "Python" / "src"))

from unreal_mcp.bridge import UEBridge  # noqa: E402


def pytest_configure(config):
    config.addinivalue_line("markers", "live: needs a running Unreal Editor with the UnrealMCP plugin")


def _port_file() -> Path | None:
    project = os.environ.get("UE_PROJECT_PATH")
    candidates = []
    if project:
        candidates.append(Path(project) / "Saved" / "UnrealMCP" / "port.txt")
    candidates.append(PLUGIN.parents[1] / "Saved" / "UnrealMCP" / "port.txt")
    for c in candidates:
        if c.exists():
            return c
    return None


@pytest.fixture
def live_call():
    """``live_call(command, params) -> response``: one connection per call, on its own event
    loop, so tests never share asyncio state (a bridge outlives the loop it connected on)."""
    port_file = _port_file()
    if port_file is None:
        pytest.skip("no running editor (Saved/UnrealMCP/port.txt not found)")
    os.environ.setdefault("UE_PROJECT_PATH", str(port_file.parents[2]))

    def call(command: str, params: dict | None = None) -> dict:
        async def go():
            bridge = UEBridge()
            try:
                return await bridge.send_command(command, params or {})
            finally:
                await bridge.disconnect()

        return asyncio.run(go())

    return call

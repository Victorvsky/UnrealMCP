# Copyright (c) 2026 victorvksy. All rights reserved.

"""Async TCP bridge client for communicating with the UnrealMCP UE5 plugin."""

import asyncio
import json
import os
from pathlib import Path


class BridgeError(Exception):
    """Raised when the bridge encounters an error."""


class UEBridge:
    """Async TCP client that connects to the UnrealMCP plugin inside Unreal Editor."""

    def __init__(self):
        self._reader: asyncio.StreamReader | None = None
        self._writer: asyncio.StreamWriter | None = None
        self._lock = asyncio.Lock()

    def _find_port_file(self) -> Path:
        """Locate the port.txt file written by the C++ plugin."""
        project_path = os.environ.get("UE_PROJECT_PATH", "")
        if project_path:
            port_file = Path(project_path) / "Saved" / "UnrealMCP" / "port.txt"
            if port_file.exists():
                return port_file

        # Try common relative paths
        for candidate in [
            Path(__file__).resolve().parents[4] / "Saved" / "UnrealMCP" / "port.txt",
            Path.cwd() / "Saved" / "UnrealMCP" / "port.txt",
        ]:
            if candidate.exists():
                return candidate

        raise BridgeError(
            "Could not find Saved/UnrealMCP/port.txt. "
            "Is the Unreal Editor running with the UnrealMCP plugin? "
            "Set UE_PROJECT_PATH env var to your project root."
        )

    async def connect(self) -> None:
        """Connect to the UE5 TCP server."""
        if self._writer is not None:
            return

        port_file = self._find_port_file()
        port = int(port_file.read_text().strip())

        try:
            self._reader, self._writer = await asyncio.open_connection("127.0.0.1", port)
        except ConnectionRefusedError:
            raise BridgeError(
                f"Connection refused on port {port}. "
                "Is the Unreal Editor running with the UnrealMCP plugin loaded?"
            )

    async def disconnect(self) -> None:
        """Disconnect from the UE5 TCP server."""
        if self._writer:
            self._writer.close()
            try:
                await self._writer.wait_closed()
            except Exception:
                pass
            self._writer = None
            self._reader = None

    async def send_command(self, command: str, params: dict | None = None) -> dict:
        """Send a command to UE5 and return the JSON response.

        Args:
            command: The command name (e.g. "list_actors").
            params: Optional parameters dict.

        Returns:
            The response dict from UE5.

        Raises:
            BridgeError: On connection or protocol errors.
        """
        async with self._lock:
            await self.connect()

            msg = json.dumps({"command": command, "params": params or {}}) + "\n"

            try:
                self._writer.write(msg.encode("utf-8"))
                await self._writer.drain()

                line = await asyncio.wait_for(self._reader.readline(), timeout=60.0)
                if not line:
                    # Connection closed, try to reconnect once
                    await self.disconnect()
                    await self.connect()
                    self._writer.write(msg.encode("utf-8"))
                    await self._writer.drain()
                    line = await asyncio.wait_for(self._reader.readline(), timeout=60.0)
                    if not line:
                        raise BridgeError("Connection closed by UE5")

                response = json.loads(line.decode("utf-8"))
                return response

            except asyncio.TimeoutError:
                await self.disconnect()
                raise BridgeError("Timeout waiting for UE5 response (60s)")
            except (ConnectionError, OSError) as e:
                await self.disconnect()
                raise BridgeError(f"Connection error: {e}")


# Global bridge instance shared by all routers
bridge = UEBridge()

# Copyright (c) 2026 victorvksy. All rights reserved.

"""Transport-level tools: connectivity check."""

from mcp.types import Tool

from unreal_mcp.bridge import bridge


def get_tools() -> list[Tool]:
    return [
        Tool(
            name="ping",
            description=(
                "Check that the Unreal Editor bridge is reachable. Optionally echoes the length "
                "of a payload string, which proves large and non-ASCII messages survive the trip."
            ),
            inputSchema={
                "type": "object",
                "properties": {
                    "payload": {
                        "type": "string",
                        "description": "Optional string; its character count and last character are echoed back.",
                    },
                },
            },
        ),
    ]


async def handle_ping(args: dict) -> str:
    params = {}
    if args.get("payload") is not None:
        params["payload"] = args["payload"]
    resp = await bridge.send_command("ping", params)
    if "error" in resp:
        return f"Error: {resp['error']}"
    if "payload_length" in resp:
        return f"pong (payload {int(resp['payload_length'])} chars, tail {resp.get('payload_tail', '')!r})"
    return "pong"


def get_handlers() -> dict:
    return {"ping": handle_ping}

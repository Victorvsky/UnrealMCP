# Copyright (c) 2026 victorvksy. All rights reserved.

"""capture_viewport tool contract, with the editor mocked out: image content first, then the
state as JSON text; structured errors come back as text."""

import asyncio
import base64
import json

from mcp.types import ImageContent, TextContent

from unreal_mcp.routers import capture

JPEG_MAGIC = b"\xff\xd8\xff"


def _fake_response():
    return {
        "success": True,
        "timestamp": 12.5,
        "timestamp_kind": "world_time",
        "camera": {"location": {"x": 1, "y": 2, "z": 3}, "rotation": {"pitch": 0, "yaw": 90, "roll": 0}, "fov": 90, "world": "pie"},
        "actors": [{"name": "BP_Alder", "class": "BP_Alder_C", "screen_bbox": [10, 20, 110, 220], "world_location": {"x": 0, "y": 0, "z": 0}, "distance": 300.0}],
        "image": {"format": "jpeg", "width": 1024, "height": 576, "mime_type": "image/jpeg", "bytes": 3,
                  "data": base64.b64encode(JPEG_MAGIC).decode()},
        "timings": {"game_thread_capture_ms": 4.0, "encode_ms": 2.0},
    }


def test_capture_returns_image_then_state(monkeypatch):
    async def fake_send(command, params=None):
        assert command == "capture_viewport"
        assert params == {"camera": "pie", "resolution": {"w": 1024, "h": 576}}
        return _fake_response()

    monkeypatch.setattr(capture.bridge, "send_command", fake_send)
    content = asyncio.run(capture.handle_capture_viewport({"camera": "pie", "resolution": {"w": 1024, "h": 576}}))
    assert isinstance(content[0], ImageContent)
    assert content[0].mimeType == "image/jpeg"
    assert base64.b64decode(content[0].data).startswith(JPEG_MAGIC)
    assert isinstance(content[1], TextContent)
    meta = json.loads(content[1].text)
    assert meta["camera"]["world"] == "pie"
    assert meta["actors"][0]["name"] == "BP_Alder"
    assert meta["actors"][0]["screen_bbox"] == [10, 20, 110, 220]
    assert "data" not in meta["image"] and meta["image"]["width"] == 1024


def test_structured_error_is_text(monkeypatch):
    async def fake_send(command, params=None):
        return {"success": False, "error": "pie_not_running: No Play-In-Editor session is running (Call start_pie first)",
                "error_detail": {"code": "pie_not_running"}}

    monkeypatch.setattr(capture.bridge, "send_command", fake_send)
    content = asyncio.run(capture.handle_capture_viewport({"camera": "pie"}))
    assert len(content) == 1 and isinstance(content[0], TextContent)
    assert content[0].text.startswith("Error: pie_not_running")


def test_optional_params_are_only_sent_when_given(monkeypatch):
    seen = {}

    async def fake_send(command, params=None):
        seen.update(params)
        return _fake_response()

    monkeypatch.setattr(capture.bridge, "send_command", fake_send)
    asyncio.run(capture.handle_capture_viewport({}))
    assert seen == {}

# Copyright (c) 2026 victorvksy. All rights reserved.

"""Recording tool contracts with the editor mocked out: parameters pass through untouched,
get_recording_frames yields a header plus image/text pairs for the available frames only,
and structured errors come back as text."""

import asyncio
import base64
import json

from mcp.types import ImageContent, TextContent

from unreal_mcp.routers import recording

JPEG_MAGIC = b"\xff\xd8\xff"


def test_record_pie_passes_only_given_params(monkeypatch):
    seen = {}

    async def fake_send(command, params=None):
        assert command == "record_pie"
        seen.update(params)
        return {"success": True, "session_id": "20260915-120000-abc123", "state": "recording", "manifest_uri": "file:///x/manifest.json"}

    monkeypatch.setattr(recording.bridge, "send_command", fake_send)
    text = asyncio.run(recording.handle_record_pie({"duration_s": 10, "fps": 2, "start_pie": False, "actor_filter": ["Pawn"]}))
    assert seen == {"duration_s": 10, "fps": 2, "start_pie": False, "actor_filter": ["Pawn"]}
    assert json.loads(text)["session_id"] == "20260915-120000-abc123"
    assert "success" not in json.loads(text)


def test_frames_content_is_header_then_image_text_pairs(monkeypatch):
    async def fake_send(command, params=None):
        assert command == "get_recording_frames"
        assert params == {"session_id": "s1", "start_s": 0, "end_s": 1, "fps": 4}
        return {
            "success": True,
            "session_id": "s1",
            "recorded_fps": 2,
            "requested_fps": 4,
            "truncated": False,
            "available": 2,
            "frames": [
                {"t": 0.0, "available": True, "index": 0, "recorded_t": 0.0, "world_time": 100.0,
                 "image": {"format": "jpeg", "mime_type": "image/jpeg", "width": 320, "height": 180, "bytes": 3, "data": base64.b64encode(JPEG_MAGIC).decode()},
                 "actors": [{"name": "BP_Alder", "screen_bbox": [1, 2, 3, 4]}]},
                {"t": 0.25, "available": False, "reason": "not available: no recorded frame near this time"},
                {"t": 0.5, "available": True, "index": 1, "recorded_t": 0.5, "world_time": 100.5,
                 "image": {"format": "jpeg", "mime_type": "image/jpeg", "width": 320, "height": 180, "bytes": 3, "data": base64.b64encode(JPEG_MAGIC).decode()},
                 "actors": []},
            ],
        }

    monkeypatch.setattr(recording.bridge, "send_command", fake_send)
    content = asyncio.run(recording.handle_get_recording_frames({"session_id": "s1", "start_s": 0, "end_s": 1, "fps": 4}))
    assert isinstance(content[0], TextContent)
    header = json.loads(content[0].text)
    assert header["grid"] == [
        {"t": 0.0, "available": True, "index": 0},
        {"t": 0.25, "available": False, "reason": "not available: no recorded frame near this time"},
        {"t": 0.5, "available": True, "index": 1},
    ]
    assert len(content) == 1 + 2 * 2
    assert isinstance(content[1], ImageContent) and base64.b64decode(content[1].data).startswith(JPEG_MAGIC)
    meta = json.loads(content[2].text)
    assert meta["actors"][0]["name"] == "BP_Alder" and "data" not in meta["image"] and meta["t"] == 0.0
    assert isinstance(content[3], ImageContent) and isinstance(content[4], TextContent)


def test_structured_errors_are_text(monkeypatch):
    async def fake_send(command, params=None):
        return {"success": False, "error": "session_not_found: No session nope (Call record_pie first)", "error_detail": {"code": "session_not_found"}}

    monkeypatch.setattr(recording.bridge, "send_command", fake_send)
    assert asyncio.run(recording.handle_get_recording_status({"session_id": "nope"})).startswith("Error: session_not_found")
    frames = asyncio.run(recording.handle_get_recording_frames({"session_id": "nope"}))
    assert len(frames) == 1 and frames[0].text.startswith("Error: session_not_found")
    assert asyncio.run(recording.handle_get_recording_timeline({"session_id": "nope"})).startswith("Error: session_not_found")
    assert asyncio.run(recording.handle_stop_recording({})).startswith("Error: session_not_found")


def test_manifest_resource_stays_inside_the_storage_root(tmp_path, monkeypatch):
    root = tmp_path / "Saved" / "MCPRecordings"
    (root / "s1").mkdir(parents=True)
    (root / "s1" / "manifest.json").write_text('{"session_id": "s1"}', encoding="utf-8")
    (tmp_path / "secret.json").write_text("no", encoding="utf-8")
    monkeypatch.setenv("UE_PROJECT_PATH", str(tmp_path))
    listed = recording.list_manifests()
    assert len(listed) == 1 and listed[0]["name"] == "recording s1"
    assert json.loads(recording.read_manifest(listed[0]["uri"]))["session_id"] == "s1"
    assert recording.read_manifest((tmp_path / "secret.json").resolve().as_uri()) is None

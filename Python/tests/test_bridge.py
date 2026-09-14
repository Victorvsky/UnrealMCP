# Copyright (c) 2026 victorvksy. All rights reserved.

"""Bridge behaviour that does not need an editor: error normalization, line limit, framing."""

import asyncio
import json

from unreal_mcp import bridge as bridge_mod
from unreal_mcp.bridge import UEBridge, normalize_error, MAX_LINE_BYTES


def test_structured_error_is_flattened_and_kept():
    resp = normalize_error({"success": False, "error": {"code": "busy", "message": "still running", "hint": "retry"}})
    assert resp["error"] == "busy: still running (retry)"
    assert resp["error_detail"] == {"code": "busy", "message": "still running", "hint": "retry"}


def test_string_error_passes_through():
    resp = normalize_error({"error": "Unknown command: x"})
    assert resp["error"] == "Unknown command: x"
    assert "error_detail" not in resp


def test_success_untouched():
    resp = normalize_error({"success": True, "reply": "pong"})
    assert resp == {"success": True, "reply": "pong"}


def test_line_limit_covers_multi_megabyte_responses():
    assert MAX_LINE_BYTES >= 16 * 1024 * 1024


class _FakeReader:
    def __init__(self, line: bytes):
        self._line = line

    async def readline(self):
        line, self._line = self._line, b""
        return line


class _FakeWriter:
    def __init__(self):
        self.sent = b""

    def write(self, data):
        self.sent += data

    async def drain(self):
        pass

    def close(self):
        pass

    async def wait_closed(self):
        pass


def test_send_command_frames_one_json_line_and_normalizes(monkeypatch):
    b = UEBridge()
    reply = json.dumps({"success": False, "error": {"code": "timeout", "message": "slow"}}).encode() + b"\n"
    b._reader, b._writer = _FakeReader(reply), _FakeWriter()

    async def fake_connect(self):
        pass

    monkeypatch.setattr(UEBridge, "connect", fake_connect)
    resp = asyncio.run(b.send_command("ping", {"payload": "é→"}))
    sent = b._writer.sent
    assert sent.endswith(b"\n") and sent.count(b"\n") == 1
    assert json.loads(sent.decode("utf-8")) == {"command": "ping", "params": {"payload": "é→"}}
    assert resp["error"] == "timeout: slow"
    assert resp["error_detail"]["code"] == "timeout"

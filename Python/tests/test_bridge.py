# Copyright (c) 2026 victorvksy. All rights reserved.

"""Bridge behaviour that does not need an editor: error normalization, framing, reconnect."""

import asyncio
import json

from unreal_mcp.bridge import UEBridge, normalize_error, is_idempotent


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


class _DroppingReader:
    """First readline returns EOF (connection dropped); later ones return the given line."""

    def __init__(self, line: bytes):
        self._line = line
        self.calls = 0

    async def readline(self):
        self.calls += 1
        if self.calls == 1:
            return b""
        return self._line


def _wire_reconnect(monkeypatch, reader, writer):
    async def fake_connect(self):
        self._reader, self._writer = reader, writer

    async def fake_disconnect(self):
        self._reader = self._writer = None

    monkeypatch.setattr(UEBridge, "connect", fake_connect)
    monkeypatch.setattr(UEBridge, "disconnect", fake_disconnect)


def test_dropped_connection_does_not_resend_a_mutating_command(monkeypatch):
    # The command may already have run on the game thread; resending spawn_actor would
    # execute it twice. Reconnect for the next call, report connection_lost, do not resend.
    reader, writer = _DroppingReader(b'{"success": true}\n'), _FakeWriter()
    _wire_reconnect(monkeypatch, reader, writer)
    resp = asyncio.run(UEBridge().send_command("spawn_actor", {"class_name": "Cube"}))
    assert resp["error_detail"]["code"] == "connection_lost"
    assert "check state" in resp["error"]
    assert writer.sent.count(b"\n") == 1, "spawn_actor must be written exactly once"
    assert reader.calls == 1


def test_dropped_connection_retries_an_idempotent_command_once(monkeypatch):
    reader, writer = _DroppingReader(b'{"success": true, "actors": []}\n'), _FakeWriter()
    _wire_reconnect(monkeypatch, reader, writer)
    resp = asyncio.run(UEBridge().send_command("list_actors", {}))
    assert resp == {"success": True, "actors": []}
    assert writer.sent.count(b"\n") == 2


def test_idempotent_allowlist_is_explicit():
    assert is_idempotent("ping") and is_idempotent("list_actors") and is_idempotent("get_property")
    assert not is_idempotent("spawn_actor") and not is_idempotent("set_property") and not is_idempotent("delete_asset")

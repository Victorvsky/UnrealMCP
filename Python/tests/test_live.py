# Copyright (c) 2026 victorvksy. All rights reserved.

"""Round trips against a running editor. Skipped when no editor is up.

    pytest -m live Python/tests
"""

import pytest

pytestmark = pytest.mark.live


def test_ping(live_call):
    resp = live_call("ping")
    assert resp.get("success") is True and resp.get("reply") == "pong", resp


def test_five_megabyte_round_trip_with_multibyte_tail(live_call):
    # The receive path used to convert byte counts as character counts, which truncated any
    # non-ASCII message; and lines over 64 KB used to overflow the Python reader.
    payload = "".join(chr(ord("a") + (i % 26)) for i in range(5 * 1024 * 1024)) + "é→"
    resp = live_call("ping", {"payload": payload})
    assert resp.get("success") is True, str(resp)[:300]
    assert int(resp["payload_length"]) == len(payload)
    assert resp["payload_tail"] == "→"


def test_unknown_command_is_a_structured_error(live_call):
    resp = live_call("__no_such_command__")
    assert resp["error_detail"]["code"] == "unknown_command", resp
    assert resp["error"].startswith("unknown_command:")


def test_large_response_survives_the_reader_limit(live_call):
    # list_blueprints on a mid-size project is well over the old 64 KB default.
    resp = live_call("list_blueprints")
    assert "error" not in resp, str(resp)[:300]

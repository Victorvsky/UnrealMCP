# Copyright (c) 2026 victorvksy. All rights reserved.

"""The packaged copy under Content/Python must match Python/src (see tools/sync_content.py)."""

import sys
from pathlib import Path

sys.path.insert(0, str(Path(__file__).resolve().parents[1] / "tools"))

from sync_content import divergence  # noqa: E402


def test_content_copy_matches_source():
    diff = divergence()
    assert not diff, "run `python Python/tools/sync_content.py`:\n  " + "\n  ".join(diff)

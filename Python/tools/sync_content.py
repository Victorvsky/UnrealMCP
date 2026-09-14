# Copyright (c) 2026 victorvksy. All rights reserved.

"""Keep the packaged copy of the server in sync with the source of truth.

``Python/src/unreal_mcp`` is where the server is developed. ``Content/Python/unreal_mcp`` is
the copy that ships inside the plugin (next to a vendored ``mcp`` SDK under
``Content/Python/Lib``, which this script never touches).

    python Python/tools/sync_content.py          # copy src -> Content
    python Python/tools/sync_content.py --check  # exit 1 if the two differ (used by the tests)
"""

from __future__ import annotations

import filecmp
import shutil
import sys
from pathlib import Path

PLUGIN = Path(__file__).resolve().parents[2]
SRC = PLUGIN / "Python" / "src" / "unreal_mcp"
DST = PLUGIN / "Content" / "Python" / "unreal_mcp"
IGNORE = {"__pycache__", ".pytest_cache"}


def _files(root: Path) -> dict[str, Path]:
    return {
        str(p.relative_to(root)).replace("\\", "/"): p
        for p in root.rglob("*")
        if p.is_file() and not (set(p.relative_to(root).parts) & IGNORE) and p.suffix != ".pyc"
    }


def divergence() -> list[str]:
    """Relative paths that differ between src and Content (missing on either side counts)."""
    if not DST.exists():
        return ["<Content/Python/unreal_mcp is missing>"]
    a, b = _files(SRC), _files(DST)
    out = []
    for rel in sorted(set(a) | set(b)):
        if rel not in a:
            out.append(f"only in Content: {rel}")
        elif rel not in b:
            out.append(f"only in src: {rel}")
        elif not filecmp.cmp(a[rel], b[rel], shallow=False):
            out.append(f"differs: {rel}")
    return out


def sync() -> None:
    if DST.exists():
        shutil.rmtree(DST)
    shutil.copytree(SRC, DST, ignore=shutil.ignore_patterns(*IGNORE, "*.pyc"))


def main(argv: list[str]) -> int:
    if "--check" in argv:
        diff = divergence()
        if diff:
            print("Content/Python/unreal_mcp is out of sync with Python/src/unreal_mcp:")
            for line in diff:
                print("  " + line)
            print("Run: python Python/tools/sync_content.py")
            return 1
        print("Content/Python/unreal_mcp is in sync.")
        return 0
    sync()
    print(f"Synced {SRC} -> {DST}")
    return 0


if __name__ == "__main__":
    sys.exit(main(sys.argv[1:]))

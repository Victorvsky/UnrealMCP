# Copyright (c) 2026 victorvksy. All rights reserved.

"""Sequencer tools — list, read, edit LevelSequence tracks, bindings, and playback ranges."""

from mcp.types import Tool

from unreal_mcp.bridge import bridge


def get_tools() -> list[Tool]:
    return [
        Tool(
            name="list_sequences",
            description=(
                "List all LevelSequence assets in the project. "
                "Returns name, path, track count, and duration."
            ),
            inputSchema={
                "type": "object",
                "properties": {
                    "path_filter": {
                        "type": "string",
                        "description": "Substring to filter sequence paths.",
                    },
                },
            },
        ),
        Tool(
            name="read_sequence",
            description=(
                "Read detailed info about a LevelSequence: all tracks (master and per-binding), "
                "bindings (possessables and spawnables), playback range, and display rate."
            ),
            inputSchema={
                "type": "object",
                "properties": {
                    "sequence": {
                        "type": "string",
                        "description": "Sequence name or full asset path.",
                    },
                },
                "required": ["sequence"],
            },
        ),
        Tool(
            name="add_sequence_track",
            description=(
                "Add a track to a LevelSequence. Can be a master track or bound to a specific actor binding. "
                "Supported types: Float, Bool, Transform, Visibility, Event, Audio, SkeletalAnimation, Sub, CameraCut, Fade."
            ),
            inputSchema={
                "type": "object",
                "properties": {
                    "sequence": {
                        "type": "string",
                        "description": "Sequence name or path.",
                    },
                    "track_type": {
                        "type": "string",
                        "description": "Track type: Float, Bool, Transform, Visibility, Event, Audio, SkeletalAnimation, Sub, CameraCut, Fade.",
                        "enum": ["Float", "Bool", "Transform", "Visibility", "Event", "Audio", "SkeletalAnimation", "Sub", "CameraCut", "Fade"],
                    },
                    "name": {
                        "type": "string",
                        "description": "Optional display name for the track.",
                    },
                    "binding_guid": {
                        "type": "string",
                        "description": "Optional: GUID of a binding to attach this track to (from read_sequence). If omitted, adds as master track.",
                    },
                },
                "required": ["sequence", "track_type"],
            },
        ),
        Tool(
            name="remove_sequence_track",
            description="Remove a track from a LevelSequence by display name.",
            inputSchema={
                "type": "object",
                "properties": {
                    "sequence": {
                        "type": "string",
                        "description": "Sequence name or path.",
                    },
                    "track_name": {
                        "type": "string",
                        "description": "Display name of the track to remove.",
                    },
                    "binding_guid": {
                        "type": "string",
                        "description": "Optional: GUID of the binding the track belongs to.",
                    },
                },
                "required": ["sequence", "track_name"],
            },
        ),
        Tool(
            name="set_sequence_playback_range",
            description="Set the playback range (start and end time in seconds) of a LevelSequence.",
            inputSchema={
                "type": "object",
                "properties": {
                    "sequence": {
                        "type": "string",
                        "description": "Sequence name or path.",
                    },
                    "start_seconds": {
                        "type": "number",
                        "description": "Start time in seconds.",
                    },
                    "end_seconds": {
                        "type": "number",
                        "description": "End time in seconds.",
                    },
                },
                "required": ["sequence", "start_seconds", "end_seconds"],
            },
        ),
        Tool(
            name="add_sequence_binding",
            description=(
                "Bind a level actor to a LevelSequence as a possessable. "
                "Returns the binding GUID needed to add tracks to this actor."
            ),
            inputSchema={
                "type": "object",
                "properties": {
                    "sequence": {
                        "type": "string",
                        "description": "Sequence name or path.",
                    },
                    "actor_name": {
                        "type": "string",
                        "description": "Name or label of the actor in the level to bind.",
                    },
                },
                "required": ["sequence", "actor_name"],
            },
        ),
    ]


def get_handlers() -> dict:
    return {
        "list_sequences": handle_list_sequences,
        "read_sequence": handle_read_sequence,
        "add_sequence_track": handle_add_sequence_track,
        "remove_sequence_track": handle_remove_sequence_track,
        "set_sequence_playback_range": handle_set_sequence_playback_range,
        "add_sequence_binding": handle_add_sequence_binding,
    }


async def handle_list_sequences(args: dict) -> str:
    params = {}
    if "path_filter" in args:
        params["path_filter"] = args["path_filter"]

    resp = await bridge.send_command("list_sequences", params)
    if "error" in resp:
        return f"Error: {resp['error']}"

    seqs = resp.get("sequences", [])
    if not seqs:
        return "No LevelSequences found."

    lines = [f"Found {resp['count']} sequences:\n"]
    for s in seqs:
        dur = s.get("duration_seconds")
        dur_str = f", {dur:.1f}s" if dur is not None else ""
        tracks = s.get("track_count", "?")
        lines.append(f"  - {s['name']} ({tracks} tracks{dur_str})")
        lines.append(f"    {s['path']}")
    return "\n".join(lines)


async def handle_read_sequence(args: dict) -> str:
    resp = await bridge.send_command("read_sequence", {"sequence": args["sequence"]})
    if "error" in resp:
        return f"Error: {resp['error']}"

    lines = [
        f"Sequence: {resp.get('name', '')}",
        f"Path: {resp.get('path', '')}",
        f"Display Rate: {resp.get('display_rate', '?')}",
    ]

    start = resp.get("start_seconds")
    end = resp.get("end_seconds")
    dur = resp.get("duration_seconds")
    if start is not None and end is not None:
        lines.append(f"Playback Range: {start:.2f}s — {end:.2f}s (duration: {dur:.2f}s)")

    # Bindings
    bindings = resp.get("bindings", [])
    if bindings:
        lines.append(f"\nBindings ({resp.get('binding_count', 0)}):")
        for b in bindings:
            btype = b.get("type", "?")
            bclass = b.get("class", "")
            class_str = f" [{bclass}]" if bclass else ""
            lines.append(f"  - {b['name']} ({btype}{class_str})")
            lines.append(f"    GUID: {b.get('guid', '?')}")
            tracks = b.get("tracks", [])
            for t in tracks:
                lines.append(f"      Track: {t['name']} ({t['type']}, {t.get('section_count', 0)} sections)")

    # Master tracks
    tracks = resp.get("tracks", [])
    master_tracks = [t for t in tracks if t.get("scope") == "master"]
    if master_tracks:
        lines.append(f"\nMaster Tracks ({len(master_tracks)}):")
        for t in master_tracks:
            lines.append(f"  - {t['name']} ({t['type']}, {t.get('section_count', 0)} sections)")

    return "\n".join(lines)


async def handle_add_sequence_track(args: dict) -> str:
    params = {
        "sequence": args["sequence"],
        "track_type": args["track_type"],
    }
    if "name" in args:
        params["name"] = args["name"]
    if "binding_guid" in args:
        params["binding_guid"] = args["binding_guid"]

    resp = await bridge.send_command("add_sequence_track", params)
    if "error" in resp:
        return f"Error: {resp['error']}"

    binding_info = ""
    if "binding_guid" in resp:
        binding_info = f" (bound to {resp['binding_guid']})"
    return (
        f"Added {resp.get('track_type', '')} track '{resp.get('track_name', '')}' "
        f"to sequence '{resp.get('sequence', '')}'{binding_info}"
    )


async def handle_remove_sequence_track(args: dict) -> str:
    params = {
        "sequence": args["sequence"],
        "track_name": args["track_name"],
    }
    if "binding_guid" in args:
        params["binding_guid"] = args["binding_guid"]

    resp = await bridge.send_command("remove_sequence_track", params)
    if "error" in resp:
        return f"Error: {resp['error']}"

    return f"Removed track '{resp.get('removed_track', '')}' from sequence '{resp.get('sequence', '')}'"


async def handle_set_sequence_playback_range(args: dict) -> str:
    resp = await bridge.send_command("set_sequence_playback_range", {
        "sequence": args["sequence"],
        "start_seconds": args["start_seconds"],
        "end_seconds": args["end_seconds"],
    })
    if "error" in resp:
        return f"Error: {resp['error']}"

    return (
        f"Set playback range on '{resp.get('sequence', '')}': "
        f"{resp.get('start_seconds', 0):.2f}s — {resp.get('end_seconds', 0):.2f}s "
        f"(duration: {resp.get('duration_seconds', 0):.2f}s)"
    )


async def handle_add_sequence_binding(args: dict) -> str:
    resp = await bridge.send_command("add_sequence_binding", {
        "sequence": args["sequence"],
        "actor_name": args["actor_name"],
    })
    if "error" in resp:
        return f"Error: {resp['error']}"

    return (
        f"Bound actor '{resp.get('actor', '')}' to sequence '{resp.get('sequence', '')}' "
        f"as {resp.get('type', 'possessable')} (GUID: {resp.get('binding_guid', '')})"
    )

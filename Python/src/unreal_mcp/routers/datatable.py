# Copyright (c) 2026 victorvksy. All rights reserved.

"""DataTable tools — list, read, add, edit, and remove rows in UE5 DataTables."""

from mcp.types import Tool

from unreal_mcp.bridge import bridge


def get_tools() -> list[Tool]:
    return [
        Tool(
            name="list_data_tables",
            description=(
                "List all DataTable assets in the project. "
                "Returns name, path, row struct type, and row count for each."
            ),
            inputSchema={
                "type": "object",
                "properties": {
                    "path_filter": {
                        "type": "string",
                        "description": "Substring to filter DataTable paths.",
                    },
                },
            },
        ),
        Tool(
            name="read_data_table",
            description=(
                "Read the full contents of a DataTable: column definitions and all rows. "
                "Use row_filter to search for specific rows by name."
            ),
            inputSchema={
                "type": "object",
                "properties": {
                    "data_table": {
                        "type": "string",
                        "description": "DataTable name (e.g. 'DT_Items') or full path.",
                    },
                    "row_filter": {
                        "type": "string",
                        "description": "Optional: only return rows whose name contains this substring.",
                    },
                },
                "required": ["data_table"],
            },
        ),
        Tool(
            name="add_data_table_row",
            description=(
                "Add a new row to a DataTable. "
                "The row_name must be unique. Values are provided as column_name: value pairs."
            ),
            inputSchema={
                "type": "object",
                "properties": {
                    "data_table": {
                        "type": "string",
                        "description": "DataTable name or path.",
                    },
                    "row_name": {
                        "type": "string",
                        "description": "Unique name/key for the new row.",
                    },
                    "values": {
                        "type": "object",
                        "description": "Column values as {column_name: value_string} pairs.",
                        "additionalProperties": {"type": "string"},
                    },
                },
                "required": ["data_table", "row_name"],
            },
        ),
        Tool(
            name="remove_data_table_row",
            description="Remove a row from a DataTable by name.",
            inputSchema={
                "type": "object",
                "properties": {
                    "data_table": {
                        "type": "string",
                        "description": "DataTable name or path.",
                    },
                    "row_name": {
                        "type": "string",
                        "description": "Name of the row to remove.",
                    },
                },
                "required": ["data_table", "row_name"],
            },
        ),
        Tool(
            name="edit_data_table_row",
            description=(
                "Edit an existing row in a DataTable. "
                "Only specified columns are updated; others remain unchanged."
            ),
            inputSchema={
                "type": "object",
                "properties": {
                    "data_table": {
                        "type": "string",
                        "description": "DataTable name or path.",
                    },
                    "row_name": {
                        "type": "string",
                        "description": "Name of the row to edit.",
                    },
                    "values": {
                        "type": "object",
                        "description": "Column values to update as {column_name: value_string} pairs.",
                        "additionalProperties": {"type": "string"},
                    },
                },
                "required": ["data_table", "row_name", "values"],
            },
        ),
    ]


def get_handlers() -> dict:
    return {
        "list_data_tables": handle_list_data_tables,
        "read_data_table": handle_read_data_table,
        "add_data_table_row": handle_add_data_table_row,
        "remove_data_table_row": handle_remove_data_table_row,
        "edit_data_table_row": handle_edit_data_table_row,
    }


async def handle_list_data_tables(args: dict) -> str:
    params = {}
    if "path_filter" in args:
        params["path_filter"] = args["path_filter"]

    resp = await bridge.send_command("list_data_tables", params)
    if "error" in resp:
        return f"Error: {resp['error']}"

    tables = resp.get("data_tables", [])
    if not tables:
        return "No DataTables found."

    lines = [f"Found {resp['count']} DataTables:\n"]
    for dt in tables:
        struct = dt.get("row_struct", "Unknown")
        count = dt.get("row_count", "?")
        lines.append(f"  - {dt['name']} ({struct}, {count} rows)")
        lines.append(f"    {dt['path']}")
    return "\n".join(lines)


async def handle_read_data_table(args: dict) -> str:
    params = {"data_table": args["data_table"]}
    if "row_filter" in args:
        params["row_filter"] = args["row_filter"]

    resp = await bridge.send_command("read_data_table", params)
    if "error" in resp:
        return f"Error: {resp['error']}"

    lines = [
        f"DataTable: {resp.get('data_table', '')}",
        f"Row Struct: {resp.get('row_struct', '')}",
    ]

    # Columns
    columns = resp.get("columns", [])
    if columns:
        lines.append(f"\nColumns ({len(columns)}):")
        for col in columns:
            lines.append(f"  - {col['name']}: {col['type']}")

    # Rows
    rows = resp.get("rows", [])
    lines.append(f"\nRows ({resp.get('row_count', 0)}):")
    if not rows:
        lines.append("  (no rows)")
    else:
        col_names = [c["name"] for c in columns]
        for row in rows:
            lines.append(f"  [{row.get('row_name', '?')}]")
            for col_name in col_names:
                val = row.get(col_name, "")
                if val:
                    lines.append(f"    {col_name}: {val}")

    return "\n".join(lines)


async def handle_add_data_table_row(args: dict) -> str:
    params = {
        "data_table": args["data_table"],
        "row_name": args["row_name"],
    }
    if "values" in args:
        params["values"] = args["values"]

    resp = await bridge.send_command("add_data_table_row", params)
    if "error" in resp:
        return f"Error: {resp['error']}"

    return (
        f"Added row '{resp.get('row_name', '')}' to {resp.get('data_table', '')} "
        f"(now {resp.get('row_count', '?')} rows)"
    )


async def handle_remove_data_table_row(args: dict) -> str:
    resp = await bridge.send_command("remove_data_table_row", {
        "data_table": args["data_table"],
        "row_name": args["row_name"],
    })
    if "error" in resp:
        return f"Error: {resp['error']}"

    return (
        f"Removed row '{resp.get('removed_row', '')}' from {resp.get('data_table', '')} "
        f"({resp.get('row_count', '?')} rows remaining)"
    )


async def handle_edit_data_table_row(args: dict) -> str:
    resp = await bridge.send_command("edit_data_table_row", {
        "data_table": args["data_table"],
        "row_name": args["row_name"],
        "values": args["values"],
    })
    if "error" in resp:
        return f"Error: {resp['error']}"

    fields = resp.get("updated_fields", [])
    return (
        f"Updated row '{resp.get('row_name', '')}' in {resp.get('data_table', '')} "
        f"({resp.get('fields_updated', 0)} fields: {', '.join(fields)})"
    )

# Copyright (c) 2026 victorvksy. All rights reserved.

"""Landscape & foliage tools — sculpt terrain, paint layers, and scatter foliage."""

from mcp.types import Tool

from unreal_mcp.bridge import bridge


def get_tools() -> list[Tool]:
    return [
        Tool(
            name="get_landscape_info",
            description=(
                "Get information about the landscape in the current level: dimensions, "
                "component count, bounds, landscape material, and existing paint layers. "
                "Use this to understand the landscape before sculpting or painting."
            ),
            inputSchema={
                "type": "object",
                "properties": {},
            },
        ),
        Tool(
            name="create_landscape",
            description=(
                "Create a new flat landscape with the given dimensions. "
                "size_x and size_y are the number of components in each direction. "
                "Each component is sections_per_component * quads_per_section quads wide. "
                "Default is 1 section of 63 quads per component (6300 cm at default scale). "
                "Optionally assign a landscape material by path."
            ),
            inputSchema={
                "type": "object",
                "properties": {
                    "size_x": {
                        "type": "integer",
                        "description": "Number of components in X direction.",
                    },
                    "size_y": {
                        "type": "integer",
                        "description": "Number of components in Y direction.",
                    },
                    "sections_per_component": {
                        "type": "integer",
                        "description": "Sections per component (default 1). Valid: 1 or 2.",
                    },
                    "quads_per_section": {
                        "type": "integer",
                        "description": "Quads per section (default 63). Valid: 7, 15, 31, 63, 127, 255.",
                    },
                    "material_path": {
                        "type": "string",
                        "description": "Optional full path to a landscape material asset.",
                    },
                },
                "required": ["size_x", "size_y"],
            },
        ),
        Tool(
            name="sculpt_landscape",
            description=(
                "Sculpt the landscape heightmap by raising, lowering, flattening, or "
                "smoothing terrain in a circular or square region with smooth falloff. "
                "Use this to create hills, valleys, paths, plateaus, and terrain features. "
                "Strength is in heightmap units (roughly 1 = 1/128 cm at default scale). "
                "Values of 500-5000 produce noticeable terrain changes."
            ),
            inputSchema={
                "type": "object",
                "properties": {
                    "center_x": {"type": "number", "description": "World X coordinate of sculpt center."},
                    "center_y": {"type": "number", "description": "World Y coordinate of sculpt center."},
                    "radius": {"type": "number", "description": "Sculpt radius in world units (cm)."},
                    "strength": {
                        "type": "number",
                        "description": "Sculpt strength in heightmap units. 500-5000 for moderate changes.",
                    },
                    "shape": {
                        "type": "string",
                        "enum": ["circle", "square"],
                        "description": "Shape of the sculpt region (default: circle).",
                    },
                    "operation": {
                        "type": "string",
                        "enum": ["raise", "lower", "flatten", "smooth"],
                        "description": "Sculpt operation (default: raise).",
                    },
                },
                "required": ["center_x", "center_y", "radius", "strength"],
            },
        ),
        Tool(
            name="sculpt_landscape_noise",
            description=(
                "Apply Perlin noise-based sculpting to a landscape region to create "
                "natural-looking terrain — rolling hills, rocky surfaces, valleys. "
                "Amplitude controls height variation, frequency controls feature size "
                "(lower = broader hills, higher = finer detail), octaves add detail layers. "
                "Use seed for reproducible results."
            ),
            inputSchema={
                "type": "object",
                "properties": {
                    "center_x": {"type": "number", "description": "World X coordinate of noise center."},
                    "center_y": {"type": "number", "description": "World Y coordinate of noise center."},
                    "radius": {"type": "number", "description": "Radius of the noise region in world units."},
                    "amplitude": {
                        "type": "number",
                        "description": "Height variation in heightmap units. 1000-5000 for moderate terrain.",
                    },
                    "frequency": {
                        "type": "number",
                        "description": "Noise frequency (default 0.01). Lower = broader hills, higher = finer.",
                    },
                    "octaves": {
                        "type": "integer",
                        "description": "Number of noise octaves for detail (default 4, max 8).",
                    },
                    "seed": {
                        "type": "number",
                        "description": "Random seed for reproducible noise patterns.",
                    },
                },
                "required": ["center_x", "center_y", "radius", "amplitude"],
            },
        ),
        Tool(
            name="set_landscape_height_at",
            description=(
                "Set the exact height at a specific world coordinate on the landscape. "
                "Height is in world units (cm). Useful for precise single-point edits."
            ),
            inputSchema={
                "type": "object",
                "properties": {
                    "x": {"type": "number", "description": "World X coordinate."},
                    "y": {"type": "number", "description": "World Y coordinate."},
                    "height": {"type": "number", "description": "Desired height in world units (cm)."},
                },
                "required": ["x", "y", "height"],
            },
        ),
        Tool(
            name="get_landscape_height_at",
            description=(
                "Read the landscape surface height at a world position using a line trace. "
                "Returns the Z height in world units. Use this to check terrain elevation."
            ),
            inputSchema={
                "type": "object",
                "properties": {
                    "x": {"type": "number", "description": "World X coordinate."},
                    "y": {"type": "number", "description": "World Y coordinate."},
                },
                "required": ["x", "y"],
            },
        ),
        Tool(
            name="import_heightmap",
            description=(
                "Import a raw heightmap file (.r16 or .raw, uint16 per pixel) and apply "
                "it to the existing landscape. The file dimensions must match the width "
                "and height parameters. Each pixel is a uint16 height value (32768 = zero)."
            ),
            inputSchema={
                "type": "object",
                "properties": {
                    "file_path": {
                        "type": "string",
                        "description": "Absolute path to the heightmap file (.r16 or .raw).",
                    },
                    "width": {"type": "integer", "description": "Heightmap width in pixels."},
                    "height": {"type": "integer", "description": "Heightmap height in pixels."},
                },
                "required": ["file_path", "width", "height"],
            },
        ),
        Tool(
            name="paint_landscape_layer",
            description=(
                "Paint a landscape material layer (e.g. grass, rock, dirt) in a circular "
                "region with configurable falloff. Strength 0-1 controls paint opacity. "
                "The layer must already exist — use add_landscape_layer to create one first."
            ),
            inputSchema={
                "type": "object",
                "properties": {
                    "layer_name": {
                        "type": "string",
                        "description": "Name of the landscape paint layer to apply.",
                    },
                    "center_x": {"type": "number", "description": "World X coordinate of paint center."},
                    "center_y": {"type": "number", "description": "World Y coordinate of paint center."},
                    "radius": {"type": "number", "description": "Paint radius in world units (cm)."},
                    "strength": {
                        "type": "number",
                        "description": "Paint strength from 0.0 to 1.0.",
                    },
                    "falloff": {
                        "type": "number",
                        "description": "Falloff ratio 0.0-1.0 (default 0.5). Higher = more gradual edges.",
                    },
                },
                "required": ["layer_name", "center_x", "center_y", "radius", "strength"],
            },
        ),
        Tool(
            name="add_landscape_layer",
            description=(
                "Add a new paint layer to the landscape. Creates a ULandscapeLayerInfoObject "
                "for the layer. Use this before paint_landscape_layer if the layer doesn't exist yet."
            ),
            inputSchema={
                "type": "object",
                "properties": {
                    "layer_name": {
                        "type": "string",
                        "description": "Name for the new layer (e.g. 'Grass', 'Rock', 'Dirt').",
                    },
                    "layer_info_path": {
                        "type": "string",
                        "description": "Optional path to an existing ULandscapeLayerInfoObject asset.",
                    },
                },
                "required": ["layer_name"],
            },
        ),
        Tool(
            name="place_foliage",
            description=(
                "Scatter foliage instances (grass, trees, rocks) on the landscape surface "
                "in a circular region. Instances are randomly positioned within the radius, "
                "with random scale between min/max and random yaw rotation. "
                "Uses line traces to place on the surface. Provide the full asset path "
                "to a StaticMesh (e.g. '/Game/Meshes/SM_Tree')."
            ),
            inputSchema={
                "type": "object",
                "properties": {
                    "mesh_path": {
                        "type": "string",
                        "description": "Full path to a StaticMesh asset to use as foliage.",
                    },
                    "center_x": {"type": "number", "description": "World X coordinate of scatter center."},
                    "center_y": {"type": "number", "description": "World Y coordinate of scatter center."},
                    "radius": {"type": "number", "description": "Scatter radius in world units (cm)."},
                    "count": {"type": "integer", "description": "Number of instances to place."},
                    "min_scale": {
                        "type": "number",
                        "description": "Minimum random scale (default 1.0).",
                    },
                    "max_scale": {
                        "type": "number",
                        "description": "Maximum random scale (default 1.0).",
                    },
                    "align_to_surface": {
                        "type": "boolean",
                        "description": "Align foliage to surface normal (default true).",
                    },
                },
                "required": ["mesh_path", "center_x", "center_y", "radius", "count"],
            },
        ),
        Tool(
            name="clear_foliage",
            description=(
                "Remove foliage instances in a circular region. Optionally filter by mesh "
                "path to only clear a specific foliage type. If mesh_path is omitted, "
                "clears all foliage types in the region."
            ),
            inputSchema={
                "type": "object",
                "properties": {
                    "center_x": {"type": "number", "description": "World X coordinate of clear center."},
                    "center_y": {"type": "number", "description": "World Y coordinate of clear center."},
                    "radius": {"type": "number", "description": "Clear radius in world units (cm)."},
                    "mesh_path": {
                        "type": "string",
                        "description": "Optional mesh path to clear only a specific foliage type.",
                    },
                },
                "required": ["center_x", "center_y", "radius"],
            },
        ),
    ]


def get_handlers() -> dict:
    return {
        "get_landscape_info": handle_get_landscape_info,
        "create_landscape": handle_create_landscape,
        "sculpt_landscape": handle_sculpt_landscape,
        "sculpt_landscape_noise": handle_sculpt_landscape_noise,
        "set_landscape_height_at": handle_set_landscape_height_at,
        "get_landscape_height_at": handle_get_landscape_height_at,
        "import_heightmap": handle_import_heightmap,
        "paint_landscape_layer": handle_paint_landscape_layer,
        "add_landscape_layer": handle_add_landscape_layer,
        "place_foliage": handle_place_foliage,
        "clear_foliage": handle_clear_foliage,
    }


async def handle_get_landscape_info(args: dict) -> str:
    resp = await bridge.send_command("landscape_info", {})

    if "error" in resp:
        return f"Error: {resp['error']}"

    lines = [
        f"Landscape: {resp['name']}",
        f"Components: {resp['component_count']}",
    ]

    if "component_size_quads" in resp:
        lines.append(
            f"Component size: {resp['component_size_quads']} quads "
            f"({resp['num_subsections']} subsections x {resp['subsection_size_quads']} quads)"
        )

    bounds = resp.get("bounds")
    if bounds:
        lines.append(
            f"Bounds: ({bounds['min_x']:.0f}, {bounds['min_y']:.0f}, {bounds['min_z']:.0f}) to "
            f"({bounds['max_x']:.0f}, {bounds['max_y']:.0f}, {bounds['max_z']:.0f})"
        )

    if resp.get("material"):
        lines.append(f"Material: {resp['material']}")

    layers = resp.get("layers", [])
    if layers:
        lines.append(f"Paint layers ({len(layers)}):")
        for layer in layers:
            info = f" -> {layer['info_path']}" if layer.get("info_path") else ""
            lines.append(f"  - {layer['name']}{info}")
    else:
        lines.append("Paint layers: none")

    return "\n".join(lines)


async def handle_create_landscape(args: dict) -> str:
    params = {
        "size_x": args["size_x"],
        "size_y": args["size_y"],
    }
    if "sections_per_component" in args:
        params["sections_per_component"] = args["sections_per_component"]
    if "quads_per_section" in args:
        params["quads_per_section"] = args["quads_per_section"]
    if "material_path" in args:
        params["material_path"] = args["material_path"]

    resp = await bridge.send_command("create_landscape", params)

    if "error" in resp:
        return f"Error creating landscape: {resp['error']}"

    return (
        f"Created landscape '{resp['name']}'\n"
        f"  Resolution: {resp['resolution_x']}x{resp['resolution_y']} vertices\n"
        f"  Components: {resp['components_x']}x{resp['components_y']}"
    )


async def handle_sculpt_landscape(args: dict) -> str:
    params = {
        "center_x": args["center_x"],
        "center_y": args["center_y"],
        "radius": args["radius"],
        "strength": args["strength"],
    }
    if "shape" in args:
        params["shape"] = args["shape"]
    if "operation" in args:
        params["operation"] = args["operation"]

    resp = await bridge.send_command("sculpt_landscape", params)

    if "error" in resp:
        return f"Error sculpting: {resp['error']}"

    return (
        f"Sculpted landscape ({resp['operation']})\n"
        f"  Modified {resp['modified_pixels']} pixels in {resp['region_width']}x{resp['region_height']} region"
    )


async def handle_sculpt_landscape_noise(args: dict) -> str:
    params = {
        "center_x": args["center_x"],
        "center_y": args["center_y"],
        "radius": args["radius"],
        "amplitude": args["amplitude"],
    }
    if "frequency" in args:
        params["frequency"] = args["frequency"]
    if "octaves" in args:
        params["octaves"] = args["octaves"]
    if "seed" in args:
        params["seed"] = args["seed"]

    resp = await bridge.send_command("sculpt_noise", params)

    if "error" in resp:
        return f"Error applying noise: {resp['error']}"

    return (
        f"Applied noise sculpting\n"
        f"  Modified {resp['modified_pixels']} pixels\n"
        f"  Amplitude: {resp['amplitude']}, Frequency: {resp['frequency']}, Octaves: {resp['octaves']}"
    )


async def handle_set_landscape_height_at(args: dict) -> str:
    resp = await bridge.send_command("set_height_at", {
        "x": args["x"],
        "y": args["y"],
        "height": args["height"],
    })

    if "error" in resp:
        return f"Error setting height: {resp['error']}"

    # Include diagnostic fields if present
    diag = ""
    for key in ("before_raw", "after_raw", "landscape_x", "landscape_y", "components_found", "has_layers", "can_have_layers", "scale", "location"):
        if key in resp:
            diag += f"\n  {key}: {resp[key]}"
    return f"Set height at ({resp['x']:.1f}, {resp['y']:.1f}) to {resp['height']:.1f} (raw: {resp['raw_value']}){diag}"


async def handle_get_landscape_height_at(args: dict) -> str:
    resp = await bridge.send_command("get_height_at", {
        "x": args["x"],
        "y": args["y"],
    })

    if "error" in resp:
        return f"Error reading height: {resp['error']}"

    return f"Height at ({resp['x']:.1f}, {resp['y']:.1f}) = {resp['height']:.2f} cm"


async def handle_import_heightmap(args: dict) -> str:
    resp = await bridge.send_command("import_heightmap", {
        "file_path": args["file_path"],
        "width": args["width"],
        "height": args["height"],
    })

    if "error" in resp:
        return f"Error importing heightmap: {resp['error']}"

    return (
        f"Imported heightmap from {resp['file']}\n"
        f"  Size: {resp['width']}x{resp['height']} ({resp['pixels_imported']} pixels)"
    )


async def handle_paint_landscape_layer(args: dict) -> str:
    params = {
        "layer_name": args["layer_name"],
        "center_x": args["center_x"],
        "center_y": args["center_y"],
        "radius": args["radius"],
        "strength": args["strength"],
    }
    if "falloff" in args:
        params["falloff"] = args["falloff"]

    resp = await bridge.send_command("paint_layer", params)

    if "error" in resp:
        return f"Error painting layer: {resp['error']}"

    return f"Painted layer '{resp['layer']}' — modified {resp['modified_pixels']} pixels"


async def handle_add_landscape_layer(args: dict) -> str:
    params = {"layer_name": args["layer_name"]}
    if "layer_info_path" in args:
        params["layer_info_path"] = args["layer_info_path"]

    resp = await bridge.send_command("add_layer", params)

    if "error" in resp:
        return f"Error adding layer: {resp['error']}"

    info = f" ({resp['layer_info_path']})" if resp.get("layer_info_path") else ""
    return f"Added layer '{resp['layer_name']}' at index {resp['layer_index']}{info}"


async def handle_place_foliage(args: dict) -> str:
    params = {
        "mesh_path": args["mesh_path"],
        "center_x": args["center_x"],
        "center_y": args["center_y"],
        "radius": args["radius"],
        "count": args["count"],
    }
    if "min_scale" in args:
        params["min_scale"] = args["min_scale"]
    if "max_scale" in args:
        params["max_scale"] = args["max_scale"]
    if "align_to_surface" in args:
        params["align_to_surface"] = args["align_to_surface"]

    resp = await bridge.send_command("place_foliage", params)

    if "error" in resp:
        return f"Error placing foliage: {resp['error']}"

    return (
        f"Placed {resp['placed_count']}/{resp['requested_count']} foliage instances\n"
        f"  Mesh: {resp['mesh']}"
    )


async def handle_clear_foliage(args: dict) -> str:
    params = {
        "center_x": args["center_x"],
        "center_y": args["center_y"],
        "radius": args["radius"],
    }
    if "mesh_path" in args:
        params["mesh_path"] = args["mesh_path"]

    resp = await bridge.send_command("clear_foliage", params)

    if "error" in resp:
        return f"Error clearing foliage: {resp['error']}"

    filter_msg = f" (mesh: {resp['mesh_filter']})" if resp.get("mesh_filter") else ""
    return f"Removed {resp['removed_count']} foliage instances{filter_msg}"

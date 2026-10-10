"""Create a rough, texture-free stone material in Unreal Editor.

Reuses an existing asset by default, so running the map generator again does not
replace manual material edits. Call create_stone_material(rebuild=True) to reset
its graph explicitly. No external images or third-party plugins are required.
"""

import unreal


MATERIAL_PACKAGE_PATH = "/Game/Materials/M_MonumentalStone"


# Surface-gradient bump mapping. The material uses world-space normals, while
# the noise itself follows each block in local space when a door leaf rotates.
STONE_NORMAL_CODE = """
float3 N = normalize(SurfaceNormal);
float3 dPdx = ddx(Position);
float3 dPdy = ddy(Position);
float3 R1 = cross(dPdy, N);
float3 R2 = cross(N, dPdx);
float Det = dot(dPdx, R1);
float3 Gradient = ddx(Height) * R1 + ddy(Height) * R2;
return normalize(N - sign(Det) * Gradient * Strength / max(abs(Det), 0.000001));
""".strip()


def create_stone_material(rebuild=False):
    """Return the saved Material asset; keep an existing graph unless requested."""
    assets = unreal.EditorAssetLibrary
    editing = unreal.MaterialEditingLibrary

    if assets.does_asset_exist(MATERIAL_PACKAGE_PATH):
        material = unreal.load_asset(MATERIAL_PACKAGE_PATH)
        if not isinstance(material, unreal.Material):
            raise RuntimeError(
                "{} exists but is not a Material.".format(MATERIAL_PACKAGE_PATH)
            )
        if not rebuild:
            return material
    else:
        assets.make_directory("/Game/Materials")
        material = unreal.AssetToolsHelpers.get_asset_tools().create_asset(
            "M_MonumentalStone",
            "/Game/Materials",
            unreal.Material,
            unreal.MaterialFactoryNew(),
        )
        if material is None:
            raise RuntimeError("Could not create {}.".format(MATERIAL_PACKAGE_PATH))

    editing.delete_all_material_expressions(material)
    material.set_editor_property("tangent_space_normal", False)

    def node(expression_class, x, y, **properties):
        expression = editing.create_material_expression(material, expression_class, x, y)
        if expression is None:
            raise RuntimeError("Could not create material node {}.".format(expression_class))
        for name, value in properties.items():
            expression.set_editor_property(name, value)
        return expression

    def wire(source, destination, input_name):
        if not editing.connect_material_expressions(source, "", destination, input_name):
            raise RuntimeError("Could not connect material input {}.".format(input_name))

    def output(expression, material_property):
        if not editing.connect_material_property(expression, "", material_property):
            raise RuntimeError("Could not connect material output {}.".format(material_property))

    color = node(
        unreal.MaterialExpressionVectorParameter,
        -1000, -400,
        parameter_name="Color",
        default_value=unreal.LinearColor(0.34, 0.33, 0.30, 1.0),
    )
    roughness = node(
        unreal.MaterialExpressionScalarParameter,
        -600, 250,
        parameter_name="Roughness",
        default_value=0.94,
    )
    bump_strength = node(
        unreal.MaterialExpressionScalarParameter,
        -600, 800,
        parameter_name="BumpStrength",
        default_value=1.2,
    )

    # Convert the 100 cm cube's local coordinates into physical centimetres.
    # Compute in the vertex shader, then interpolate for pixel-shader noise;
    # this also keeps vertex-only ObjectScale out of pixel-shader inputs.
    local_position = node(unreal.MaterialExpressionPreSkinnedLocalPosition, -1800, 0)
    object_scale = node(unreal.MaterialExpressionObjectScale, -1800, 200)
    scaled_position = node(unreal.MaterialExpressionMultiply, -1600, 0)
    wire(local_position, scaled_position, "A")
    wire(object_scale, scaled_position, "B")
    stone_position = node(unreal.MaterialExpressionVertexInterpolator, -1400, 0)
    wire(scaled_position, stone_position, "Input")

    weathering = node(
        unreal.MaterialExpressionNoise,
        -1200, -200,
        scale=0.009,
        quality=1,
        levels=3,
        output_min=0.72,
        output_max=1.06,
    )
    grain = node(
        unreal.MaterialExpressionNoise,
        -1200, 100,
        scale=0.30,
        quality=1,
        levels=2,
        output_min=0.0,
        output_max=1.0,
    )
    wire(stone_position, weathering, "Position")
    wire(stone_position, grain, "Position")

    grain_tint = node(
        unreal.MaterialExpressionLinearInterpolate,
        -800, -50,
        const_a=0.82,
        const_b=1.0,
    )
    wire(grain, grain_tint, "Alpha")
    weathered_color = node(unreal.MaterialExpressionMultiply, -600, -350)
    wire(color, weathered_color, "A")
    wire(weathering, weathered_color, "B")
    base_color = node(unreal.MaterialExpressionMultiply, -350, -250)
    wire(weathered_color, base_color, "A")
    wire(grain_tint, base_color, "B")
    output(base_color, unreal.MaterialProperty.MP_BASE_COLOR)

    grain_roughness = node(
        unreal.MaterialExpressionLinearInterpolate,
        -800, 400,
        const_a=0.90,
        const_b=1.06,
    )
    wire(grain, grain_roughness, "Alpha")
    surface_roughness = node(unreal.MaterialExpressionMultiply, -350, 250)
    wire(roughness, surface_roughness, "A")
    wire(grain_roughness, surface_roughness, "B")
    clamped_roughness = node(
        unreal.MaterialExpressionClamp,
        -150, 250,
        min_default=0.0,
        max_default=1.0,
    )
    wire(surface_roughness, clamped_roughness, "Input")
    output(clamped_roughness, unreal.MaterialProperty.MP_ROUGHNESS)

    world_position = node(unreal.MaterialExpressionWorldPosition, -800, 600)
    world_normal = node(unreal.MaterialExpressionVertexNormalWS, -800, 750)
    interpolated_normal = node(unreal.MaterialExpressionVertexInterpolator, -600, 650)
    wire(world_normal, interpolated_normal, "Input")
    bump_normal = node(
        unreal.MaterialExpressionCustom,
        -150, 600,
        code=STONE_NORMAL_CODE,
        output_type=unreal.CustomMaterialOutputType.CMOT_FLOAT3,
        inputs=[
            unreal.CustomInput(input_name=name)
            for name in ("Position", "SurfaceNormal", "Height", "Strength")
        ],
    )
    wire(world_position, bump_normal, "Position")
    wire(interpolated_normal, bump_normal, "SurfaceNormal")
    wire(grain, bump_normal, "Height")
    wire(bump_strength, bump_normal, "Strength")
    output(bump_normal, unreal.MaterialProperty.MP_NORMAL)

    # Metallic stays at the opaque material's default (zero).
    editing.recompile_material(material)
    if not assets.save_asset(MATERIAL_PACKAGE_PATH, only_if_is_dirty=False):
        raise RuntimeError("Could not save {}.".format(MATERIAL_PACKAGE_PATH))
    unreal.log("Created rough stone material: {}".format(MATERIAL_PACKAGE_PATH))
    return material


if __name__ == "__main__":
    create_stone_material()

"""Fast contract tests for the editor scripts, without an Unreal installation.

The fake editor checks graph wiring and asset/level lifecycle, not shader
compilation or rendering. Those still require running the scripts in Unreal.
"""

import runpy
import sys
import types
import unittest
from collections import namedtuple
from pathlib import Path
from unittest.mock import MagicMock, patch


ROOT = Path(__file__).resolve().parents[1]
MATERIAL_PATH = "/Game/Materials/M_MonumentalStone"
MAP_PATH = "/Game/Maps/StarterMap"


class EditorObject:
    def __init__(self, **properties):
        self.properties = properties

    def set_editor_property(self, name, value):
        self.properties[name] = value

    def get_editor_property(self, name):
        return self.properties.get(name)


class Material(EditorObject):
    def __init__(self):
        super().__init__()
        self.nodes = []
        self.wires = []
        self.outputs = {}


class Actor(EditorObject):
    def __init__(self):
        super().__init__()
        self.components = {}
        self.label = ""

    def set_actor_label(self, label):
        self.label = label

    def get_component_by_class(self, component_class):
        return self.components.setdefault(component_class, component_class())


class MapBase(Actor):
    def __init__(self):
        super().__init__()
        self.material_assignments = []

    def set_door_stone_material(self, material):
        self.properties["door_stone_material"] = material
        self.material_assignments.append(material)


def make_fake_editor():
    unreal = types.ModuleType("unreal")
    unreal.assets = {}
    unreal.actors = []
    unreal.Material = Material
    unreal.MaterialFactoryNew = EditorObject
    unreal.CustomInput = EditorObject
    unreal.Vector = namedtuple("Vector", "x y z")
    unreal.Rotator = namedtuple("Rotator", "pitch yaw roll")
    unreal.LinearColor = namedtuple("LinearColor", "r g b a")
    unreal.ComponentMobility = types.SimpleNamespace(MOVABLE="movable")
    unreal.MaterialProperty = types.SimpleNamespace(
        MP_BASE_COLOR="base_color", MP_ROUGHNESS="roughness", MP_NORMAL="normal"
    )
    unreal.CustomMaterialOutputType = types.SimpleNamespace(CMOT_FLOAT3="float3")
    for name in ("PlayerStart", "DirectionalLight", "SkyLight", "SkyAtmosphere"):
        setattr(unreal, name, type(name, (Actor,), {}))
    for name in ("DirectionalLightComponent", "SkyLightComponent", "GameMapsSettings"):
        setattr(unreal, name, type(name, (EditorObject,), {}))

    expression_names = (
        "VectorParameter", "ScalarParameter", "PreSkinnedLocalPosition",
        "ObjectScale", "VertexInterpolator", "Multiply", "Noise",
        "LinearInterpolate", "Clamp", "WorldPosition", "VertexNormalWS", "Custom",
    )
    for name in expression_names:
        name = "MaterialExpression" + name
        setattr(unreal, name, type(name, (EditorObject,), {}))

    def create_asset(name, folder, asset_class, factory):
        material = asset_class()
        unreal.assets[folder + "/" + name] = material
        return material

    def create_node(material, expression_class, x, y):
        expression = expression_class()
        expression.material = material
        material.nodes.append(expression)
        return expression

    def clear_graph(material):
        material.nodes.clear()
        material.wires.clear()
        material.outputs.clear()

    def connect(source, output_name, destination, input_name):
        valid_inputs = {
            "MaterialExpressionVertexInterpolator": {"Input"},
            "MaterialExpressionMultiply": {"A", "B"},
            "MaterialExpressionNoise": {"Position", "FilterWidth"},
            "MaterialExpressionLinearInterpolate": {"A", "B", "Alpha"},
            "MaterialExpressionClamp": {"Input", "Min", "Max"},
        }.get(type(destination).__name__, set())
        if isinstance(destination, unreal.MaterialExpressionCustom):
            valid_inputs = {
                item.properties["input_name"]
                for item in destination.properties["inputs"]
            }
        if input_name not in valid_inputs:
            return False
        destination.material.wires.append((source, destination, input_name))
        return True

    def output(expression, output_name, material_property):
        expression.material.outputs[material_property] = expression
        return True

    unreal.MaterialEditingLibrary = types.SimpleNamespace(
        create_material_expression=MagicMock(side_effect=create_node),
        delete_all_material_expressions=MagicMock(side_effect=clear_graph),
        connect_material_expressions=MagicMock(side_effect=connect),
        connect_material_property=MagicMock(side_effect=output),
        recompile_material=MagicMock(),
    )
    unreal.EditorAssetLibrary = types.SimpleNamespace(
        does_asset_exist=MagicMock(side_effect=lambda path: path in unreal.assets),
        make_directory=MagicMock(return_value=True),
        save_asset=MagicMock(return_value=True),
    )
    unreal.asset_tools = types.SimpleNamespace(create_asset=MagicMock(side_effect=create_asset))
    unreal.AssetToolsHelpers = types.SimpleNamespace(
        get_asset_tools=MagicMock(return_value=unreal.asset_tools)
    )
    unreal.load_asset = MagicMock(side_effect=lambda path: unreal.assets.get(path))
    unreal.load_class = MagicMock(return_value=MapBase)

    def new_level(path, partitioned):
        unreal.assets[path] = "level"
        unreal.actors.clear()
        return True

    def spawn_actor(actor_class, location, rotation):
        actor = actor_class()
        actor.location = location
        actor.rotation = rotation
        unreal.actors.append(actor)
        return actor

    unreal.EditorLevelLibrary = types.SimpleNamespace(
        new_level=MagicMock(side_effect=new_level),
        load_level=MagicMock(return_value=True),
        get_all_level_actors=MagicMock(side_effect=lambda: list(unreal.actors)),
        spawn_actor_from_class=MagicMock(side_effect=spawn_actor),
        save_current_level=MagicMock(return_value=True),
    )
    unreal.settings = EditorObject()
    unreal.settings.save_config = MagicMock()
    unreal.get_default_object = MagicMock(return_value=unreal.settings)
    unreal.log = MagicMock()
    unreal.log_warning = MagicMock()
    return unreal


class StoneDoorScriptTests(unittest.TestCase):
    def setUp(self):
        self.unreal = make_fake_editor()
        module_patch = patch.dict(sys.modules, {"unreal": self.unreal})
        module_patch.start()
        self.addCleanup(module_patch.stop)
        self.create_material = runpy.run_path(
            str(ROOT / "Content/Python/CreateStoneMaterial.py")
        )["create_stone_material"]
        self.create_map = runpy.run_path(
            str(ROOT / "Content/Python/CreateStarterMap.py")
        )["create_starter_map"]

    def test_material_graph_has_color_roughness_normal_and_local_coordinates(self):
        material = self.create_material()
        self.assertIs(material, self.unreal.assets[MATERIAL_PATH])
        self.assertEqual(set(material.outputs), {"base_color", "roughness", "normal"})
        self.assertFalse(material.properties["tangent_space_normal"])
        parameters = {
            node.properties["parameter_name"]: node
            for node in material.nodes if "parameter_name" in node.properties
        }
        self.assertEqual(set(parameters), {"Color", "Roughness", "BumpStrength"})
        self.assertEqual(parameters["Roughness"].properties["default_value"], 0.94)
        names = {type(node).__name__ for node in material.nodes}
        self.assertIn("MaterialExpressionPreSkinnedLocalPosition", names)
        self.assertIn("MaterialExpressionVertexInterpolator", names)
        noise_inputs = [
            source for source, destination, pin in material.wires
            if isinstance(destination, self.unreal.MaterialExpressionNoise) and pin == "Position"
        ]
        self.assertEqual(len(noise_inputs), 2)
        self.assertTrue(all(isinstance(source, self.unreal.MaterialExpressionVertexInterpolator) for source in noise_inputs))
        normal_input = next(
            source for source, destination, pin in material.wires
            if destination is material.outputs["normal"] and pin == "SurfaceNormal"
        )
        self.assertIsInstance(normal_input, self.unreal.MaterialExpressionVertexInterpolator)
        self.unreal.MaterialEditingLibrary.recompile_material.assert_called_once_with(material)
        self.unreal.EditorAssetLibrary.save_asset.assert_called_once_with(
            MATERIAL_PATH, only_if_is_dirty=False
        )

    def test_existing_material_is_reused_without_changing_manual_edits(self):
        material = self.create_material()
        old_nodes = list(material.nodes)
        material.set_editor_property("artist_note", "keep this")
        self.assertIs(self.create_material(), material)
        self.assertEqual(material.nodes, old_nodes)
        self.assertEqual(material.get_editor_property("artist_note"), "keep this")
        self.unreal.asset_tools.create_asset.assert_called_once()
        self.unreal.MaterialEditingLibrary.recompile_material.assert_called_once()

    def test_explicit_rebuild_replaces_graph_without_creating_another_asset(self):
        material = self.create_material()
        old_nodes = list(material.nodes)
        self.assertIs(self.create_material(rebuild=True), material)
        self.assertEqual(len(material.nodes), len(old_nodes))
        self.assertIsNot(material.nodes[0], old_nodes[0])
        self.unreal.asset_tools.create_asset.assert_called_once()
        self.assertEqual(self.unreal.MaterialEditingLibrary.recompile_material.call_count, 2)

    def test_wrong_asset_type_is_rejected(self):
        self.unreal.assets[MATERIAL_PATH] = EditorObject()
        with self.assertRaisesRegex(RuntimeError, "not a Material"):
            self.create_material()
        self.unreal.MaterialEditingLibrary.delete_all_material_expressions.assert_not_called()

    def test_material_creation_failure_is_reported(self):
        self.unreal.asset_tools.create_asset.side_effect = None
        self.unreal.asset_tools.create_asset.return_value = None
        with self.assertRaisesRegex(RuntimeError, "Could not create"):
            self.create_material()

    def test_material_connection_failure_is_reported(self):
        connect = self.unreal.MaterialEditingLibrary.connect_material_expressions
        connect.side_effect = None
        connect.return_value = False
        with self.assertRaisesRegex(RuntimeError, "Could not connect material input"):
            self.create_material()
        self.unreal.EditorAssetLibrary.save_asset.assert_not_called()

    def test_material_save_failure_is_reported(self):
        self.unreal.EditorAssetLibrary.save_asset.return_value = False
        with self.assertRaisesRegex(RuntimeError, "Could not save"):
            self.create_material()

    def test_new_map_has_gate_material_player_and_lighting(self):
        self.create_map()
        self.assertEqual(len(self.unreal.actors), 5)
        base = next(actor for actor in self.unreal.actors if isinstance(actor, MapBase))
        self.assertIs(base.get_editor_property("door_stone_material"), self.unreal.assets[MATERIAL_PATH])
        player = next(actor for actor in self.unreal.actors if isinstance(actor, self.unreal.PlayerStart))
        self.assertEqual(player.location, self.unreal.Vector(-4000, 0, 96))
        sun = next(actor for actor in self.unreal.actors if isinstance(actor, self.unreal.DirectionalLight))
        self.assertEqual(sun.components[self.unreal.DirectionalLightComponent].properties["intensity"], 50000)
        sky = next(actor for actor in self.unreal.actors if isinstance(actor, self.unreal.SkyLight))
        self.assertTrue(sky.components[self.unreal.SkyLightComponent].properties["real_time_capture"])
        self.assertEqual(self.unreal.settings.properties["game_default_map"], MAP_PATH + ".StarterMap")

    def test_repeated_map_generation_does_not_duplicate_actors_or_materials(self):
        self.create_map()
        actors = list(self.unreal.actors)
        self.create_map()
        self.assertEqual(self.unreal.actors, actors)
        self.unreal.asset_tools.create_asset.assert_called_once()
        self.unreal.EditorLevelLibrary.new_level.assert_called_once()
        self.unreal.EditorLevelLibrary.load_level.assert_called_once_with(MAP_PATH)
        base = next(actor for actor in actors if isinstance(actor, MapBase))
        self.assertEqual(len(base.material_assignments), 1)

    def test_existing_map_preserves_custom_material_and_player_start(self):
        self.unreal.assets[MAP_PATH] = "level"
        base = MapBase()
        custom_material = Material()
        base.set_door_stone_material(custom_material)
        player = self.unreal.PlayerStart()
        player.location = self.unreal.Vector(100, 200, 96)
        self.unreal.actors.extend([base, player])
        self.create_map()
        self.assertIs(base.get_editor_property("door_stone_material"), custom_material)
        self.assertEqual(len(base.material_assignments), 1)
        self.assertEqual(player.location, self.unreal.Vector(100, 200, 96))
        self.assertEqual(sum(isinstance(actor, MapBase) for actor in self.unreal.actors), 1)
        self.assertEqual(sum(isinstance(actor, self.unreal.PlayerStart) for actor in self.unreal.actors), 1)

    def test_existing_unassigned_gate_gets_the_stone_material(self):
        self.unreal.assets[MAP_PATH] = "level"
        base = MapBase()
        self.unreal.actors.append(base)
        self.create_map()
        self.assertIs(base.get_editor_property("door_stone_material"), self.unreal.assets[MATERIAL_PATH])

    def test_missing_cpp_class_does_not_create_assets(self):
        self.unreal.load_class.return_value = None
        with self.assertRaisesRegex(RuntimeError, "Build BaseProjectEditor"):
            self.create_map()
        self.assertFalse(self.unreal.assets)
        self.unreal.EditorLevelLibrary.new_level.assert_not_called()

    def test_map_load_failure_is_reported(self):
        self.unreal.assets[MAP_PATH] = "level"
        self.unreal.EditorLevelLibrary.load_level.return_value = False
        with self.assertRaisesRegex(RuntimeError, "Could not open existing map"):
            self.create_map()

    def test_map_creation_failure_is_reported(self):
        self.unreal.EditorLevelLibrary.new_level.side_effect = None
        self.unreal.EditorLevelLibrary.new_level.return_value = False
        with self.assertRaisesRegex(RuntimeError, "Could not create map"):
            self.create_map()

    def test_actor_spawn_failure_is_reported(self):
        self.unreal.EditorLevelLibrary.spawn_actor_from_class.side_effect = None
        self.unreal.EditorLevelLibrary.spawn_actor_from_class.return_value = None
        with self.assertRaisesRegex(RuntimeError, "Could not create actor"):
            self.create_map()
        self.unreal.EditorLevelLibrary.save_current_level.assert_not_called()

    def test_map_save_failure_is_reported(self):
        self.unreal.EditorLevelLibrary.save_current_level.return_value = False
        with self.assertRaisesRegex(RuntimeError, "Could not save.*StarterMap"):
            self.create_map()

    def test_default_map_config_failure_warns_after_saving_level(self):
        self.unreal.settings.save_config.side_effect = RuntimeError("read-only config")
        self.create_map()
        self.unreal.EditorLevelLibrary.save_current_level.assert_called_once()
        self.unreal.log_warning.assert_called_once()
        self.assertIn("Maps & Modes", self.unreal.log_warning.call_args.args[0])


if __name__ == "__main__":
    unittest.main()

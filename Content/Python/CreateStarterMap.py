"""Create the editable StarterMap level for BaseProject in Unreal Editor."""

import unreal


MAP_PACKAGE_PATH = "/Game/Maps/StarterMap"
MAP_OBJECT_PATH = "/Game/Maps/StarterMap.StarterMap"
MAP_BASE_CLASS_PATH = "/Script/BaseProject.BaseProjectMapBase"


def _spawn_actor(actor_class, location, rotation, label):
    actor = unreal.EditorLevelLibrary.spawn_actor_from_class(
        actor_class, location, rotation
    )
    if actor is None:
        raise RuntimeError("Could not create actor: {}".format(label))
    try:
        actor.set_actor_label(label)
    except Exception:
        pass
    return actor


def _has_actor(actors, actor_class):
    return any(isinstance(actor, actor_class) for actor in actors)


def _configure_light(actor, component_class, intensity=None, real_time_capture=False):
    component = actor.get_component_by_class(component_class)
    if component is None:
        return

    try:
        component.set_editor_property("mobility", unreal.ComponentMobility.MOVABLE)
    except Exception as exc:
        unreal.log_warning("Could not set movable light: {}".format(exc))

    if intensity is not None:
        try:
            component.set_editor_property("intensity", intensity)
        except Exception as exc:
            unreal.log_warning("Could not set light intensity: {}".format(exc))

    if real_time_capture:
        try:
            component.set_editor_property("real_time_capture", True)
        except Exception as exc:
            unreal.log_warning("Could not enable real-time sky capture: {}".format(exc))


def _set_default_maps():
    """Use StarterMap for editor startup and standalone/packaged game launches."""
    try:
        settings = unreal.get_default_object(unreal.GameMapsSettings)
        settings.set_editor_property("editor_startup_map", MAP_OBJECT_PATH)
        settings.set_editor_property("game_default_map", MAP_OBJECT_PATH)
        settings.save_config()
    except Exception as exc:
        unreal.log_warning(
            "StarterMap was saved, but the default maps could not be updated. "
            "Set Editor Startup Map and Game Default Map to {} in Project Settings > "
            "Maps & Modes. Details: {}".format(MAP_OBJECT_PATH, exc)
        )


def create_starter_map():
    # Check the C++ class before creating a level, so a missing editor build does
    # not leave behind an empty StarterMap asset.
    map_base_class = unreal.load_class(None, MAP_BASE_CLASS_PATH)
    if map_base_class is None:
        raise RuntimeError(
            "Could not load {}. Build BaseProjectEditor in Unreal Engine 5.8.3, "
            "then reopen the project and run this script again.".format(
                MAP_BASE_CLASS_PATH
            )
        )

    asset_library = unreal.EditorAssetLibrary
    level_library = unreal.EditorLevelLibrary

    if asset_library.does_asset_exist(MAP_PACKAGE_PATH):
        if not level_library.load_level(MAP_PACKAGE_PATH):
            raise RuntimeError("Could not open existing map {}.".format(MAP_PACKAGE_PATH))
    elif not level_library.new_level(MAP_PACKAGE_PATH, True):
        raise RuntimeError("Could not create map {}.".format(MAP_PACKAGE_PATH))

    actors = level_library.get_all_level_actors()

    if not _has_actor(actors, map_base_class):
        _spawn_actor(
            map_base_class,
            unreal.Vector(0.0, 0.0, 0.0),
            unreal.Rotator(0.0, 0.0, 0.0),
            "Map Base and Stone Gate",
        )

    if not _has_actor(actors, unreal.PlayerStart):
        _spawn_actor(
            unreal.PlayerStart,
            unreal.Vector(-1500.0, 0.0, 96.0),
            unreal.Rotator(0.0, 0.0, 0.0),
            "Player Start",
        )

    # A blank level needs its own lighting. Movable lights work without a
    # separate lighting build and keep the map usable immediately in PIE.
    if not _has_actor(actors, unreal.DirectionalLight):
        sun = _spawn_actor(
            unreal.DirectionalLight,
            unreal.Vector(0.0, 0.0, 2000.0),
            unreal.Rotator(-50.0, -35.0, 0.0),
            "StarterMap Sun",
        )
        _configure_light(
            sun, unreal.DirectionalLightComponent, intensity=50000.0
        )

    if not _has_actor(actors, unreal.SkyLight):
        sky_light = _spawn_actor(
            unreal.SkyLight,
            unreal.Vector(0.0, 0.0, 1000.0),
            unreal.Rotator(0.0, 0.0, 0.0),
            "StarterMap Sky Light",
        )
        _configure_light(
            sky_light,
            unreal.SkyLightComponent,
            intensity=1.0,
            real_time_capture=True,
        )

    if not _has_actor(actors, unreal.SkyAtmosphere):
        _spawn_actor(
            unreal.SkyAtmosphere,
            unreal.Vector(0.0, 0.0, 0.0),
            unreal.Rotator(0.0, 0.0, 0.0),
            "StarterMap Sky Atmosphere",
        )

    if not level_library.save_current_level():
        raise RuntimeError("Could not save {}.umap.".format(MAP_PACKAGE_PATH))

    _set_default_maps()
    unreal.log(
        "Created and saved {}.umap with the map base, stone gate, PlayerStart, "
        "and lighting.".format(MAP_PACKAGE_PATH)
    )


create_starter_map()

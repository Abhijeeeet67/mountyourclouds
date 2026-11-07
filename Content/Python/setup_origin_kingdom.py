# Origin Kingdom Level Setup Script
# Run in Unreal Editor Python Console or via command line
# This script sets up the complete Mission 001 environment

import unreal

# =============================================================================
# CONFIGURATION
# =============================================================================

LEVEL_PATH = "/Game/Levels/Mission001/Mission001_FirstThreat"
LEVEL_NAME = "Mission001_FirstThreat"

# World settings
WORLD_SIZE = 20000.0

# Player start location (village)
PLAYER_START = unreal.Vector(-1500.0, 200.0, 150.0)
PLAYER_ROTATION = unreal.Rotator(0, 0, 0)

# Key locations
VILLAGE_CENTER = unreal.Vector(-1500.0, 200.0, 100.0)
RUINS_ENTRANCE = unreal.Vector(2500.0, 500.0, 150.0)
ENEMY_CAMP = unreal.Vector(3500.0, 0.0, 150.0)
RIVER_PATH = [
    unreal.Vector(-2000.0, -1500.0, 50.0),
    unreal.Vector(0.0, -1000.0, 50.0),
    unreal.Vector(2000.0, -500.0, 50.0),
    unreal.Vector(4000.0, 0.0, 50.0),
]

# =============================================================================
# HELPER FUNCTIONS
# =============================================================================

def create_level():
    """Create or load the mission level"""
    if unreal.EditorAssetLibrary.does_asset_exist(LEVEL_PATH):
        unreal.EditorLoadingAndSavingUtils.load_map(LEVEL_PATH)
        print(f"Loaded existing level: {LEVEL_PATH}")
    else:
        # Create new level
        level = unreal.EditorLevelLibrary.new_level(LEVEL_PATH)
        print(f"Created new level: {LEVEL_PATH}")
    return unreal.EditorLevelLibrary.get_editor_world()

def setup_lighting(world):
    """Setup cinematic lighting for Origin Kingdom"""
    # Remove existing lights
    actors = unreal.EditorLevelLibrary.get_all_level_actors()
    for actor in actors:
        if actor.get_class().get_name() in ["DirectionalLight", "SkyLight", "SkyAtmosphere", "ExponentialHeightFog", "VolumetricCloud"]:
            unreal.EditorLevelLibrary.destroy_actor(actor)

    # Directional Light (Sun)
    sun_light = unreal.EditorLevelLibrary.spawn_actor_from_class(
        unreal.DirectionalLight,
        unreal.Vector(0, 0, 0),
        unreal.Rotator(-45, -30, 0)
    )
    sun_light.set_actor_label("Sun_DirectionalLight")
    light_comp = sun_light.get_component_by_class(unreal.DirectionalLightComponent)
    light_comp.set_intensity(10.0)
    light_comp.set_light_color(unreal.LinearColor(1.0, 0.95, 0.85))
    light_comp.set_temperature(5500)
    light_comp.set_shadow_sharpen(0.5)
    light_comp.set_volumetric_scattering_intensity(5.0)
    light_comp.set_cast_shadows(True)
    light_comp.set_mobility(unreal.ComponentMobility.MOVABLE)
    light_comp.set_affects_world(True)

    # Sky Light
    sky_light = unreal.EditorLevelLibrary.spawn_actor_from_class(
        unreal.SkyLight,
        unreal.Vector(0, 0, 0),
        unreal.Rotator(0, 0, 0)
    )
    sky_light.set_actor_label("SkyLight")
    sky_comp = sky_light.get_component_by_class(unreal.SkyLightComponent)
    sky_comp.set_intensity(1.5)
    sky_comp.set_source_type(unreal.SkyLightSourceType.SLS_SpecifiedCubemap)
    sky_comp.set_mobility(unreal.ComponentMobility.MOVABLE)

    # Sky Atmosphere
    sky_atmos = unreal.EditorLevelLibrary.spawn_actor_from_class(
        unreal.SkyAtmosphere,
        unreal.Vector(0, 0, 0),
        unreal.Rotator(0, 0, 0)
    )
    sky_atmos.set_actor_label("SkyAtmosphere")
    atmos_comp = sky_atmos.get_component_by_class(unreal.SkyAtmosphereComponent)
    atmos_comp.set_sun_multiplier(1.0)
    atmos_comp.set_rayleigh_scattering(unreal.LinearColor(0.065, 0.135, 0.3))
    atmos_comp.set_mie_scattering(unreal.LinearColor(0.006, 0.006, 0.006))
    atmos_comp.set_mie_absorption(unreal.LinearColor(0.001, 0.001, 0.001))

    # Exponential Height Fog
    fog = unreal.EditorLevelLibrary.spawn_actor_from_class(
        unreal.ExponentialHeightFog,
        unreal.Vector(0, 0, 0),
        unreal.Rotator(0, 0, 0)
    )
    fog.set_actor_label("HeightFog")
    fog_comp = fog.get_component_by_class(unreal.ExponentialHeightFogComponent)
    fog_comp.set_fog_density(0.0015)
    fog_comp.set_fog_height_falloff(0.2)
    fog_comp.set_fog_height_offset(0.0)
    fog_comp.set_fog_inscattering_color(unreal.LinearColor(0.6, 0.5, 0.4))
    fog_comp.set_fully_directional_inscattering_color_distance(10000.0)
    fog_comp.set_non_directional_inscattering_color_distance(5000.0)
    fog_comp.set_start_distance(0.0)
    fog_comp.set_second_fog_data(unreal.FogData(
        fog_density=0.0005,
        fog_height_falloff=0.1,
        fog_height_offset=5000.0
    ))

    # Volumetric Clouds
    clouds = unreal.EditorLevelLibrary.spawn_actor_from_class(
        unreal.VolumetricCloud,
        unreal.Vector(0, 0, 0),
        unreal.Rotator(0, 0, 0)
    )
    clouds.set_actor_label("VolumetricClouds")
    cloud_comp = clouds.get_component_by_class(unreal.VolumetricCloudComponent)
    cloud_comp.set_layer_bottom_altitude(2000.0)
    cloud_comp.set_layer_thickness(3000.0)

    print("Lighting setup complete")

def setup_post_process(world):
    """Setup post-process volume for cinematic look"""
    actors = unreal.EditorLevelLibrary.get_all_level_actors()
    for actor in actors:
        if actor.get_class().get_name() == "PostProcessVolume":
            unreal.EditorLevelLibrary.destroy_actor(actor)

    pp_volume = unreal.EditorLevelLibrary.spawn_actor_from_class(
        unreal.PostProcessVolume,
        unreal.Vector(0, 0, 0),
        unreal.Rotator(0, 0, 0)
    )
    pp_volume.set_actor_label("GlobalPostProcess")
    pp_comp = pp_volume.get_component_by_class(unreal.PostProcessComponent)
    pp_comp.set_unbound(True)
    pp_comp.set_priority(0)

    settings = pp_comp.get_settings()
    settings.bloom_intensity = 0.8
    settings.bloom_threshold = 0.9
    settings.bloom_size_scale = 1.0
    settings.auto_exposure_min_brightness = 0.5
    settings.auto_exposure_max_brightness = 2.0
    settings.auto_exposure_speed_up = 3.0
    settings.auto_exposure_speed_down = 1.0
    settings.vignette_intensity = 0.3
    settings.film_slope = 0.9
    settings.film_toe = 0.5
    settings.film_shoulder = 0.3
    settings.film_black_clip = 0.0
    settings.film_white_clip = 0.04
    settings.scene_color_tint = unreal.LinearColor(1.0, 0.98, 0.95)
    settings.ambient_occlusion_intensity = 0.8
    settings.ambient_occlusion_radius = 200.0
    settings.ambient_occlusion_quality = 100.0
    settings.motion_blur_amount = 0.5
    settings.lens_flare_intensity = 0.3
    settings.depth_of_field_focal_distance = 2000.0
    settings.depth_of_field_fstop = 2.8
    settings.depth_of_field_sensor_width = 24.0

    print("Post-process setup complete")

def setup_player_start(world):
    """Setup player start location"""
    actors = unreal.EditorLevelLibrary.get_all_level_actors()
    for actor in actors:
        if actor.get_class().get_name() == "PlayerStart":
            unreal.EditorLevelLibrary.destroy_actor(actor)

    player_start = unreal.EditorLevelLibrary.spawn_actor_from_class(
        unreal.PlayerStart,
        PLAYER_START,
        PLAYER_ROTATION
    )
    player_start.set_actor_label("PlayerStart_Village")
    print(f"Player start created at {PLAYER_START}")

def setup_navigation(world):
    """Setup navigation mesh for AI"""
    actors = unreal.EditorLevelLibrary.get_all_level_actors()
    for actor in actors:
        if actor.get_class().get_name() == "NavMeshBoundsVolume":
            unreal.EditorLevelLibrary.destroy_actor(actor)

    nav_volume = unreal.EditorLevelLibrary.spawn_actor_from_class(
        unreal.NavMeshBoundsVolume,
        unreal.Vector(0, 0, 0),
        unreal.Rotator(0, 0, 0)
    )
    nav_volume.set_actor_label("NavMeshBounds")
    nav_volume.set_actor_scale3d(unreal.Vector(10.0, 10.0, 5.0))
    print("Navigation mesh bounds created")

def spawn_environment_actors(world):
    """Spawn environment placeholder actors (trees, rocks, buildings)"""
    import math
    import random

    random.seed(12345)

    # Spawn trees in forest areas
    tree_positions = []
    for x in range(-8000, 8000, 800):
        for y in range(-6000, 6000, 800):
            if random.random() < 0.7:
                # Add some noise
                noise_x = random.uniform(-300, 300)
                noise_y = random.uniform(-300, 300)
                pos = unreal.Vector(x + noise_x, y + noise_y, 100.0)

                # Don't spawn too close to key locations
                if (pos - VILLAGE_CENTER).size() > 500 and \
                   (pos - RUINS_ENTRANCE).size() > 400 and \
                   (pos - ENEMY_CAMP).size() > 400:
                    tree_positions.append(pos)

    print(f"Generated {len(tree_positions)} tree positions")

    # Spawn rocks/cliffs
    rock_positions = []
    for x in range(-10000, 10000, 1500):
        for y in range(-8000, 8000, 1500):
            if random.random() < 0.15:
                noise_x = random.uniform(-500, 500)
                noise_y = random.uniform(-500, 500)
                pos = unreal.Vector(x + noise_x, y + noise_y, 100.0)
                rock_positions.append(pos)

    print(f"Generated {len(rock_positions)} rock positions")

    # Village buildings (placeholder cubes)
    building_positions = [
        unreal.Vector(-1700, 400, 100),
        unreal.Vector(-1300, 400, 100),
        unreal.Vector(-1700, 0, 100),
        unreal.Vector(-1300, 0, 100),
        unreal.Vector(-1500, -200, 100),
        unreal.Vector(-1900, 200, 100),
        unreal.Vector(-1100, 200, 100),
    ]

    # Ruins structures
    ruin_positions = [
        (unreal.Vector(2500, 500, 150), unreal.Vector(2, 2, 1.5)),
        (unreal.Vector(2700, 300, 150), unreal.Vector(1.5, 1.5, 1)),
        (unreal.Vector(2300, 300, 150), unreal.Vector(1.5, 1.5, 1)),
        (unreal.Vector(2900, 500, 150), unreal.Vector(1, 1, 2)),
        (unreal.Vector(2500, 700, 150), unreal.Vector(2, 1, 1)),
    ]

    # Enemy camp structures
    camp_positions = [
        (unreal.Vector(3300, -200, 150), unreal.Vector(2, 2, 1.5)),
        (unreal.Vector(3700, -200, 150), unreal.Vector(2, 2, 1.5)),
        (unreal.Vector(3500, 200, 150), unreal.Vector(1.5, 1.5, 2)),
        (unreal.Vector(3500, -200, 150), unreal.Vector(3, 1, 2)),  # Leader tent
    ]

    print("Environment positions generated")
    return {
        "trees": tree_positions,
        "rocks": rock_positions,
        "buildings": building_positions,
        "ruins": ruin_positions,
        "camp": camp_positions
    }

def setup_mission_triggers(world):
    """Setup mission objective triggers"""
    # Objective 1: Reach ruins entrance
    obj1_trigger = unreal.EditorLevelLibrary.spawn_actor_from_class(
        unreal.TriggerBox,
        RUINS_ENTRANCE,
        unreal.Rotator(0, 0, 0)
    )
    obj1_trigger.set_actor_label("Trigger_Obj1_RuinsEntrance")
    trigger_comp = obj1_trigger.get_component_by_class(unreal.BoxComponent)
    trigger_comp.set_box_extent(unreal.Vector(300, 300, 100))

    # Objective 2: Enemy encounter area
    obj2_trigger = unreal.EditorLevelLibrary.spawn_actor_from_class(
        unreal.TriggerBox,
        unreal.Vector(3000, 200, 150),
        unreal.Rotator(0, 0, 0)
    )
    obj2_trigger.set_actor_label("Trigger_Obj2_EnemyEncounter")
    trigger_comp = obj2_trigger.get_component_by_class(unreal.BoxComponent)
    trigger_comp.set_box_extent(unreal.Vector(400, 400, 100))

    # Objective 3: Defeat leader area
    obj3_trigger = unreal.EditorLevelLibrary.spawn_actor_from_class(
        unreal.TriggerBox,
        ENEMY_CAMP,
        unreal.Rotator(0, 0, 0)
    )
    obj3_trigger.set_actor_label("Trigger_Obj3_DefeatLeader")
    trigger_comp = obj3_trigger.get_component_by_class(unreal.BoxComponent)
    trigger_comp.set_box_extent(unreal.Vector(200, 200, 100))

    # Objective 4: Return to village
    obj4_trigger = unreal.EditorLevelLibrary.spawn_actor_from_class(
        unreal.TriggerBox,
        VILLAGE_CENTER,
        unreal.Rotator(0, 0, 0)
    )
    obj4_trigger.set_actor_label("Trigger_Obj4_ReturnVillage")
    trigger_comp = obj4_trigger.get_component_by_class(unreal.BoxComponent)
    trigger_comp.set_box_extent(unreal.Vector(300, 300, 100))

    print("Mission triggers created")

def setup_audio_ambience(world):
    """Setup ambient audio actors"""
    # Forest ambience
    forest_audio = unreal.EditorLevelLibrary.spawn_actor_from_class(
        unreal.AmbientSound,
        unreal.Vector(-1000, 0, 200),
        unreal.Rotator(0, 0, 0)
    )
    forest_audio.set_actor_label("Ambience_Forest")
    # Audio component would be set with forest sounds

    # Village ambience
    village_audio = unreal.EditorLevelLibrary.spawn_actor_from_class(
        unreal.AmbientSound,
        VILLAGE_CENTER,
        unreal.Rotator(0, 0, 0)
    )
    village_audio.set_actor_label("Ambience_Village")

    # River ambience
    river_audio = unreal.EditorLevelLibrary.spawn_actor_from_class(
        unreal.AmbientSound,
        unreal.Vector(1000, -1000, 100),
        unreal.Rotator(0, 0, 0)
    )
    river_audio.set_actor_label("Ambience_River")

    # Ruins ambience
    ruins_audio = unreal.EditorLevelLibrary.spawn_actor_from_class(
        unreal.AmbientSound,
        RUINS_ENTRANCE,
        unreal.Rotator(0, 0, 0)
    )
    ruins_audio.set_actor_label("Ambience_Ruins")

    print("Audio ambience actors created")

def save_level():
    """Save the level"""
    unreal.EditorLoadingAndSavingUtils.save_map(LEVEL_PATH)
    print(f"Level saved: {LEVEL_PATH}")

# =============================================================================
# MAIN EXECUTION
# =============================================================================

def main():
    print("=" * 60)
    print("SETTING UP ORIGIN KINGDOM - MISSION 001: THE FIRST THREAT")
    print("=" * 60)

    world = create_level()
    setup_lighting(world)
    setup_post_process(world)
    setup_player_start(world)
    setup_navigation(world)
    spawn_environment_actors(world)
    setup_mission_triggers(world)
    setup_audio_ambience(world)
    save_level()

    print("=" * 60)
    print("LEVEL SETUP COMPLETE!")
    print("=" * 60)
    print(f"Level: {LEVEL_PATH}")
    print(f"Player Start: {PLAYER_START}")
    print(f"Village: {VILLAGE_CENTER}")
    print(f"Ruins: {RUINS_ENTRANCE}")
    print(f"Enemy Camp: {ENEMY_CAMP}")
    print("=" * 60)

if __name__ == "__main__":
    main()
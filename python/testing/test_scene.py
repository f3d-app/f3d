import tempfile
from pathlib import Path

import f3d


def test_scene_memory():
    testing_dir = Path(__file__).parent.parent.parent / "testing"
    reference = Path(testing_dir) / "baselines/TestPythonSceneMemory.png"
    output = Path(tempfile.gettempdir()) / "TestPythonSceneMemory.png"

    engine = f3d.Engine.create(True)
    engine.window.size = 300, 300

    engine.scene.add(
        f3d.Mesh(
            points=[0.0, 0.0, 0.0, 0.0, 1.0, 0.0, 1.0, 0.0, 0.0],
            face_sides=[3],
            face_indices=[0, 1, 2],
        )
    )

    img = engine.window.render_to_image()
    img.save(output)

    assert img.compare(f3d.Image(reference)) < 0.05


def test_scene():
    testing_dir = Path(__file__).parent.parent.parent / "testing"
    world = testing_dir / "data/world.obj"
    logo = testing_dir / "data/f3d.glb"
    sphere1 = testing_dir / "data/mb/recursive/mb_1_0.vtp"
    sphere2 = testing_dir / "data/mb/recursive/mb_2_0.vtp"
    cube = testing_dir / "data/mb/recursive/mb_0_0.vtu"
    reference = testing_dir / "baselines/TestPythonScene.png"
    reference_red_light = testing_dir / "baselines/TestPythonSceneRedLight.png"
    output = Path(tempfile.gettempdir()) / "TestPythonScene.png"

    engine = f3d.Engine.create(True)
    engine.window.size = 300, 300

    engine.scene.add([world, logo])
    engine.scene.add(sphere1)
    engine.scene.add([sphere2, cube])

    assert engine.scene.animation_time_range() == (0.0, 4.0)
    engine.scene.load_animation_time(2)

    img = engine.window.render_to_image()
    img.save(output)

    assert img.compare(f3d.Image(reference)) < 0.05

    engine.scene.remove_all_lights()
    red_light = f3d.LightState(color=f3d.Color(1.0, 0.0, 0.0))
    green_light = f3d.LightState(
        color=f3d.Color(0.0, 1.0, 0.0),
        position=(0.0, 1.0, 0.0),
        direction=(0.0, -1.0, 0.0),
        intensity=0.5,
        positional_light=True,
        switch_state=True,
        type=f3d.LightType.HEADLIGHT,
    )
    engine.scene.add_light(red_light)
    engine.scene.add_light(green_light)

    assert engine.scene.get_light_count() == 2
    assert engine.scene.get_light(0).color.to_tuple() == (1.0, 0.0, 0.0)
    assert engine.scene.get_light(1).color.to_tuple() == (0.0, 1.0, 0.0)

    blue_light = green_light
    blue_light.color = f3d.Color(0.0, 0.0, 1.0)
    blue_light.type = f3d.LightType.CAMERA_LIGHT
    engine.scene.update_light(1, blue_light)
    assert engine.scene.get_light(1).color.to_tuple() == blue_light.color.to_tuple()
    assert engine.scene.get_light(1).position == blue_light.position
    assert engine.scene.get_light(1).direction == blue_light.direction
    assert engine.scene.get_light(1).intensity == blue_light.intensity
    assert engine.scene.get_light(1).type == f3d.LightType.CAMERA_LIGHT

    engine.scene.remove_light(1)
    assert engine.scene.get_light_count() == 1

    img = engine.window.render_to_image()
    img.save(output)

    assert img.compare(f3d.Image(reference_red_light)) < 0.05


def test_scene_buffer():
    testing_dir = Path(__file__).parent.parent.parent / "testing"
    reference = Path(testing_dir) / "baselines/TestPythonSceneBuffer.png"
    output = Path(tempfile.gettempdir()) / "TestPythonSceneBuffer.png"
    points = testing_dir / "data/points.ply"

    engine = f3d.Engine.create(True)
    engine.window.size = 300, 300

    buf = Path(points).read_bytes()
    options = engine.options
    options["scene.force_reader"] = "PLYReader"
    engine.scene.add(buf, len(buf))

    img = engine.window.render_to_image()
    img.save(output)

    assert img.compare(f3d.Image(reference)) < 0.05

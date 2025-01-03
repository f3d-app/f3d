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

    result, _error = img.compare(f3d.Image(reference), 0.05)
    assert result


def test_scene():
    testing_dir = Path(__file__).parent.parent.parent / "testing"
    world = testing_dir / "data/world.obj"
    logo = testing_dir / "data/f3d.glb"
    sphere1 = testing_dir / "data/mb/recursive/mb_1_0.vtp"
    sphere2 = testing_dir / "data/mb/recursive/mb_2_0.vtp"
    cube = testing_dir / "data/mb/recursive/mb_0_0.vtu"
    reference = testing_dir / "baselines/TestPythonScene.png"
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

    result, _error = img.compare(f3d.Image(reference), 0.05)
    assert result

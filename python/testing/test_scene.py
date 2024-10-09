from pathlib import Path
import pytest
import tempfile

import f3d


def test_scene_memory():
    testing_dir = Path(__file__).parent.parent.parent / "testing"
    reference = f"{testing_dir}/baselines/TestPythonSceneMemory.png"
    output = tempfile.gettempdir() + "/TestPythonSceneMemory.png"

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

    error = 0.0

    assert img.compare(f3d.Image(reference), 0.05, error)


def test_scene():
    testing_dir = Path(__file__).parent.parent.parent / "testing"
    world = f"{testing_dir}/data/world.obj"
    logo = f"{testing_dir}/data/f3d.glb"
    sphere1 = f"{testing_dir}/data/mb/recursive/mb_1_0.vtp"
    sphere2 = f"{testing_dir}/data/mb/recursive/mb_2_0.vtp"
    cube = f"{testing_dir}/data/mb/recursive/mb_0_0.vtu"
    cube = f"{testing_dir}/data/f3d.glb"
    reference = f"{testing_dir}/baselines/TestPythonScene.png"
    output = tempfile.gettempdir() + "/TestPythonScene.png"

    engine = f3d.Engine.create(True)
    engine.window.size = 300, 300

    engine.scene.add([world, logo])
    engine.scene.add(Path(sphere1))
    engine.scene.add([Path(sphere2), Path(cube)])

    img = engine.window.render_to_image()
    img.save(output)

    error = 0.0

    assert img.compare(f3d.Image(reference), 0.05, error)

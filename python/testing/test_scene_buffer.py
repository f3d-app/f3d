import tempfile
from pathlib import Path

import f3d


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

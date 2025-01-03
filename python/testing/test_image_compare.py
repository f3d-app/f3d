from pathlib import Path
import tempfile

import f3d


def test_compare_with_file():
    testing_dir = Path(__file__).parent.parent.parent / "testing"
    dataset = testing_dir / "data/cow.vtp"
    reference = testing_dir / "baselines/TestPythonCompareWithFile.png"
    output = Path(tempfile.gettempdir()) / "TestPythonCompareWithFile.png"

    engine = f3d.Engine.create(True)
    engine.window.size = 300, 300

    # verify the size is properly set
    assert engine.window.width == 300
    assert engine.window.height == 300

    engine.scene.add(dataset)

    img = engine.window.render_to_image()
    img.save(output)

    assert img.compare(f3d.Image(reference)) < 0.05

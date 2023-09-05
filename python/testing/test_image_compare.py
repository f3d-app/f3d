import pytest
import tempfile
import f3d


def test_compare_with_file():
    dataset = "./testing/data/cow.vtp"
    reference = "./testing/baselines/TestPythonCompareWithFile.png"
    output = tempfile.gettempdir() + "/TestPythonCompareWithFile.png"
    outputDiff = tempfile.gettempdir() + "/TestPythonCompareWithFile.diff.png"

    engine = f3d.engine(f3d.window.NATIVE_OFFSCREEN)
    engine.window.size = 300, 300

    # verify the size is properly set
    assert engine.window.width == 300
    assert engine.window.height == 300

    engine.loader.load_geometry(dataset, True)

    img = engine.window.render_to_image()
    img.save(output)

    diff = f3d.image()
    error = 0.0

    assert img.compare(f3d.image(reference), 50, diff, error)

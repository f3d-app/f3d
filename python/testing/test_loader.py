from pathlib import Path
import pytest
import tempfile

import f3d


def test_load_memory():
    testing_dir = Path(__file__).parent.parent.parent / "testing"
    reference = f"{testing_dir}/baselines/TestPythonLoadMemory.png"
    output = tempfile.gettempdir() + "/TestPythonLoadMemory.png"
    outputDiff = tempfile.gettempdir() + "/TestPythonLoadMemory.diff.png"

    engine = f3d.Engine(f3d.Window.NATIVE_OFFSCREEN)
    engine.window.size = 300, 300

    engine.loader.load_geometry(
        [0.0, 0.0, 0.0, 0.0, 1.0, 0.0, 1.0, 0.0, 0.0], [3], [0, 1, 2]
    )

    img = engine.window.render_to_image()
    img.save(output)

    diff = f3d.Image()
    error = 0.0

    assert img.compare(f3d.Image(reference), 50, diff, error)

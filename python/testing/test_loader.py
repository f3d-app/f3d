import pytest
import tempfile
import f3d


def test_load_memory():
    reference = "./testing/baselines/TestSDKLoadMemory.png"
    output = tempfile.gettempdir() + "/TestSDKLoadMemory.png"
    outputDiff = tempfile.gettempdir() + "/TestSDKLoadMemory.diff.png"

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

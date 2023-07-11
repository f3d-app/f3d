import os

import pytest

import f3d
import f3d_init


@pytest.fixture
def f3d_source_dir(pytestconfig):
    return pytestconfig.getoption("f3d_source_dir")


@pytest.fixture
def cmake_binary_dir(pytestconfig):
    return pytestconfig.getoption("cmake_binary_dir")


def test_compare_with_file(f3d_source_dir, cmake_binary_dir):
    dataset = f3d_source_dir + "/testing/data/cow.vtp"
    reference = f3d_source_dir + "/testing/baselines/TestPythonCompareWithFile.png"
    output = cmake_binary_dir + "/Testing/Temporary/TestPythonCompareWithFile.png"
    outputDiff = (
        cmake_binary_dir + "/Testing/Temporary/TestPythonCompareWithFile.diff.png"
    )

    f3d.engine.autoload_plugins()

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


@pytest.mark.skipif(
    os.environ.get("F3D_NO_DEPRECATED"), reason="F3D_NO_DEPRECATED is set"
)
def test_legacy_tests(f3d_source_dir, cmake_binary_dir):
    dataset = f3d_source_dir + "/testing/data/cow.vtp"
    reference = f3d_source_dir + "/testing/baselines/TestPythonCompareWithFile.png"
    output = cmake_binary_dir + "/Testing/Temporary/TestPythonCompareWithFile.png"
    outputDiff = (
        cmake_binary_dir + "/Testing/Temporary/TestPythonCompareWithFile.diff.png"
    )

    f3d.engine.autoloadPlugins()

    engine = f3d.engine(f3d.window.NATIVE_OFFSCREEN)
    engine.getWindow().setSize(300, 300)

    # verify the size is properly set
    assert engine.getWindow().getWidth() == 300
    assert engine.getWindow().getHeight() == 300

    engine.getLoader().loadGeometry(dataset, True)

    img = engine.getWindow().renderToImage()
    img.save(output)

    diff = f3d.image()
    error = 0.0

    ret = img.compare(f3d.image(reference), 50, diff, error)

    if not ret:
        diff.save(outputDiff)

    assert ret is True

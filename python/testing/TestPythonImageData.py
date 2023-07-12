import os

import pytest

import f3d
import f3d_init


@pytest.fixture
def cmake_binary_dir(pytestconfig):
    return pytestconfig.getoption("cmake_binary_dir")


@pytest.fixture
def f3d_engine():
    engine = f3d.engine(f3d.window.NATIVE_OFFSCREEN)
    engine.window.size = 300, 200
    return engine


def test_render_rgb(f3d_engine):
    window = f3d_engine.window

    img = window.render_to_image()
    assert img.width == window.width
    assert img.height == window.height
    assert img.channel_count == 3

    data = img.content
    assert isinstance(data, (bytes, bytearray))
    assert len(data) == img.channel_count * img.width * img.height


def test_render_rgba(f3d_engine):
    window = f3d_engine.window

    img = window.render_to_image(True)
    assert img.width == window.width
    assert img.height == window.height
    assert img.channel_count == 4
    data = img.content
    assert isinstance(data, (bytes, bytearray))
    assert len(data) == img.channel_count * img.width * img.height


def test_set_data(f3d_engine):
    img = f3d_engine.window.render_to_image()
    data = img.content[:]
    img.content = data
    assert img.content == data


def test_set_wrong_data(f3d_engine):
    img = f3d_engine.window.render_to_image()
    with pytest.raises(ValueError):
        img.content = img.content[:-1]


def test_save(cmake_binary_dir, f3d_engine):
    img = f3d_engine.window.render_to_image()
    fn = cmake_binary_dir + "/Testing/Temporary/TestPythonSaveFile.bmp"

    img.save(fn, f3d.image.SaveFormat.BMP)
    assert os.path.isfile(fn)

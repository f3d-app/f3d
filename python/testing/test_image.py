import os
import tempfile
import pytest

import f3d


@pytest.fixture
def f3d_engine():
    engine = f3d.Engine(f3d.Window.NATIVE_OFFSCREEN)
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


def test_save(f3d_engine):
    img = f3d_engine.window.render_to_image()
    fn = tempfile.gettempdir() + "/TestPythonSaveFile.bmp"

    img.save(fn, f3d.Image.SaveFormat.BMP)
    assert os.path.isfile(fn)

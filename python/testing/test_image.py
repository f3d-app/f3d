import os
from pathlib import Path
import tempfile
import pytest

import f3d


@pytest.fixture
def f3d_engine():
    engine = f3d.Engine.create(True)
    engine.window.size = 300, 200
    return engine


def test_render_rgb(f3d_engine: f3d.Engine):
    window = f3d_engine.window

    img = window.render_to_image()
    assert img.width == window.width
    assert img.height == window.height
    assert img.channel_count == 3

    data = img.content
    assert isinstance(data, (bytes, bytearray))
    assert len(data) == img.channel_count * img.width * img.height


def test_render_rgba(f3d_engine: f3d.Engine):
    window = f3d_engine.window

    img = window.render_to_image(True)
    assert img.width == window.width
    assert img.height == window.height
    assert img.channel_count == 4
    data = img.content
    assert isinstance(data, (bytes, bytearray))
    assert len(data) == img.channel_count * img.width * img.height


def test_set_data(f3d_engine: f3d.Engine):
    img = f3d_engine.window.render_to_image()
    data = img.content[:]
    img.content = data
    assert img.content == data


def test_set_wrong_data(f3d_engine: f3d.Engine):
    img = f3d_engine.window.render_to_image()
    with pytest.raises(ValueError):
        img.content = img.content[:-1]


def test_save(f3d_engine: f3d.Engine):
    img = f3d_engine.window.render_to_image()
    fn = Path(tempfile.gettempdir()) / "TestPythonSaveFile.bmp"

    img.save(fn, f3d.Image.SaveFormat.BMP)
    assert os.path.isfile(fn)


def test_save_buffer(f3d_engine: f3d.Engine):
    img = f3d_engine.window.render_to_image(True)
    buffer = img.save_buffer(f3d.Image.SaveFormat.PNG)
    assert buffer.startswith(b"\x89PNG")
    assert img._repr_png_() == buffer


def test_formats(f3d_engine: f3d.Engine):
    formats = f3d.Image.supported_formats()
    assert ".png" in formats


def test_normalized_pixel(f3d_engine: f3d.Engine):
    img = f3d_engine.window.render_to_image()
    assert img.normalized_pixel((0, 0)) == [0.2, 0.2, 0.2]


@pytest.mark.parametrize(
    "img_filename",
    [
        "toTerminalText-rgb.png",
        "toTerminalText-rgba.png",
    ],
)
def test_to_terminal_text(img_filename: str):
    testing_data_dir = Path(__file__).parent.parent.parent / "testing/data"
    image_path = testing_data_dir / img_filename
    text_path = image_path.with_suffix(".txt")
    assert (
        f3d.Image(image_path).to_terminal_text()
        == open(text_path, encoding="utf8").read()
    )


def test_metadata():
    image = f3d.Image(4, 2, 3, f3d.Image.ChannelType.BYTE)
    image.set_metadata("foo", "bar")
    image.set_metadata("hello", "world")
    assert image.get_metadata("foo") == "bar" and image.get_metadata("hello") == "world"

    with pytest.raises(KeyError):
        image.get_metadata("baz")

    assert set(image.all_metadata()) == set(["foo", "hello"])

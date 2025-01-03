import f3d

from pathlib import Path


def test_text_distance():
    assert f3d.Utils.text_distance("modle", "model") == 2


def test_collapse_path():
    assert f3d.Utils.collapse_path("/folder/../file.ext", ".") == Path("/file.ext")

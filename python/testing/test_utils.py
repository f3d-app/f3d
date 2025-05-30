import f3d

from pathlib import Path


def test_text_distance():
    assert f3d.Utils.text_distance("modle", "model") == 2


def test_collapse_path():
    assert f3d.Utils.collapse_path("/folder/../file.ext", ".") == Path("/file.ext")


def test_glob_to_regex():
    assert f3d.Utils.glob_to_regex("*vt?") == "[^/]*vt[^/]"
    assert f3d.Utils.glob_to_regex("*vt?", False, "/") == ".*vt."
    assert f3d.Utils.glob_to_regex("*vt?", path_separator="\\") == "[^\\\\]*vt[^\\\\]"
    assert (
        f3d.Utils.glob_to_regex(
            glob="*gl{tf,b}", support_glob_stars=False, path_separator="/"
        )
        == ".*gl(?:tf|b)"
    )

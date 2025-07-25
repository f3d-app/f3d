import f3d
import os

from pathlib import Path


def test_text_distance():
    assert f3d.Utils.text_distance("modle", "model") == 2


def test_collapse_path():
    assert f3d.Utils.collapse_path("/folder/../file.ext", ".") == Path("/file.ext")


def test_glob_to_regex():
    assert f3d.Utils.glob_to_regex("*vt?") == ".*vt."
    assert f3d.Utils.glob_to_regex("**/*vt?") == "(?:[^/]*(?:/|$))*[^/]*vt[^/]"
    assert (
        f3d.Utils.glob_to_regex(glob="**\\\\*vt?", path_separator="\\")
        == "(?:[^\\\\]*(?:\\\\|$))*[^\\\\]*vt[^\\\\]"
    )


def test_get_env():
    assert f3d.Utils.get_env("F3D_TEST_ENV_EMPTY") == None


def test_get_know_folder():
    if os.name == "nt":
        assert f3d.Utils.get_known_folder(f3d.Utils.KnownFolder.PICTURES) != None
    else:
        assert f3d.Utils.get_known_folder(f3d.Utils.KnownFolder.PICTURES) == None

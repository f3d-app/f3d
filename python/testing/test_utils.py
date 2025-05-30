import f3d

import os

from pathlib import Path


def test_text_distance():
    assert f3d.Utils.text_distance("modle", "model") == 2


def test_collapse_path():
    assert f3d.Utils.collapse_path("/folder/../file.ext", ".") == Path("/file.ext")


def test_glob_to_regex():
    assert f3d.Utils.glob_to_regex("*vt?", False, False) == ".*vt."
    regex_sep = "/" if os.sep == "/" else "\\\\"
    assert f3d.Utils.glob_to_regex("*vt?") == f"[^{regex_sep}]*vt[^{regex_sep}]"
    assert f3d.Utils.glob_to_regex("*vt?", use_generic_separator=True) == "[^/]*vt[^/]"
    assert (
        f3d.Utils.glob_to_regex(
            glob="*gl{tf,b}", support_glob_stars=False, use_generic_separator=True
        )
        == ".*gl(?:tf|b)"
    )

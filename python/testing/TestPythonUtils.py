import os

import pytest

import f3d
import f3d_init


def test_text_distance():
    assert f3d.utils.text_distance("modle", "model") == 2


@pytest.mark.skipif(
    os.environ.get("F3D_NO_DEPRECATED"), reason="F3D_NO_DEPRECATED is set"
)
def test_legacy_tests():
    distance = f3d.utils.textDistance("modle", "model")

    assert distance == 2

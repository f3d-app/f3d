import f3d
import f3d_init


def test_text_distance():
    assert f3d.utils.text_distance("modle", "model") == 2

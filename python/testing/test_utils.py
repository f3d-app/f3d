import f3d


def test_text_distance():
    assert f3d.Utils.text_distance("modle", "model") == 2

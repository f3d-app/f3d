import pytest

import f3d


def test_color_compare():
    color = f3d.Color(0.1, 0.2, 0.3)
    assert color.compare(f3d.Color(0.1, 0.2, 0.3), 0.0)


@pytest.mark.parametrize("component", range(3))
def test_color_compare_components(component: int):
    color = f3d.Color(0.1, 0.2, 0.3)
    values = [0.1, 0.2, 0.3]
    values[component] += 0.0000005
    near = f3d.Color(*values)
    assert color.compare(near, 1e-6)
    assert near.compare(color, 1e-6)
    assert not color.compare(near, 0.0)

    values[component] += 0.000002
    assert not color.compare(f3d.Color(*values), 1e-6)


def test_color_compare_boundary():
    assert f3d.Color(0.0, 0.0, 0.0).compare(f3d.Color(0.0, 0.0, 1e-6), 1e-6)

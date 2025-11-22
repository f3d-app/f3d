from typing import Any
import pytest

import f3d


def test_closest_option():
    option, distance = f3d.Options().get_closest_option("scene_direction")
    assert option == "scene.up_direction"
    assert distance == 3


def test_setitem():
    options = f3d.Options()
    options["ui.axis"] = False
    options["render.background.blur.coc"] = 33.33
    options["scene.animation.speed_factor"] = 3.3
    options["render.raytracing.samples"] = 5
    options["render.grid.color"] = [1.0, 1.0, 1.0]
    options["model.point_sprites.type"] = "sphere"


def test_getitem():
    engine = f3d.Engine.create_none()
    options = engine.options

    assert options["ui.axis"] is False
    assert options["render.background.blur.coc"] == 20.0
    assert options["scene.animation.speed_factor"] == 1.0
    assert options["render.raytracing.samples"] == 5
    assert options["render.grid.color"] == [0.0, 0.0, 0.0]
    assert options["model.point_sprites.type"] == "sphere"


def test_get_non_existent_key():
    options = f3d.Options()
    with pytest.raises(KeyError):
        assert options["hello"] == "world"


def test_set_non_existent_key():
    options = f3d.Options()
    with pytest.raises(KeyError):
        options["hello"] = "world"


def test_set_invalid_str_value():
    options = f3d.Options()
    with pytest.raises(ValueError):
        options["ui.axis"] = "world"


def test_set_incompatible_value_type():
    options = f3d.Options()
    with pytest.raises(AttributeError):
        options["ui.axis"] = 1.12


def test_len():
    options = f3d.Options()
    assert len(options) == len(options.keys())


def test_iter():
    options = f3d.Options()
    assert list(options) == options.keys()


def test_contains():
    options = f3d.Options()
    assert "render.grid.color" in options
    assert "hello.world" not in options


def test_set_options():
    options = f3d.Options()
    options["ui.axis"] = True
    options["model.material.roughness"] = 0.7
    options["scene.animation.speed_factor"] = 3.7
    options["render.raytracing.samples"] = 2
    options["model.color.rgb"] = [0.0, 1.0, 1.0]
    options["model.point_sprites.type"] = "other"

    engine = f3d.Engine.create_none()
    engine.options = options
    assert engine.options["ui.axis"] is True
    assert engine.options["model.material.roughness"] == 0.7
    assert engine.options["scene.animation.speed_factor"] == 3.7
    assert engine.options["render.raytracing.samples"] == 2
    assert engine.options["model.color.rgb"] == [0.0, 1.0, 1.0]
    assert engine.options["model.point_sprites.type"] == "other"


def test_set_options_from_string():
    options = f3d.Options()
    options["ui.axis"] = False
    assert not options["ui.axis"]

    options["ui.axis"] = "on"
    assert options["ui.axis"]

    options["ui.axis"] = "off"
    assert not options["ui.axis"]


def test_to_dict():
    options = f3d.Options()
    options_dict = dict(options)

    assert len(options) > 0
    assert len(options) == len(options_dict)
    for k, v in options_dict.items():
        assert options[k] == v


def test_update_from_dict():
    options = f3d.Options()

    d: dict[str, Any] = {
        "ui.axis": True,
        "model.material.roughness": 0.8,
        "scene.animation.speed_factor": 3.8,
        "render.raytracing.samples": 8,
        "model.color.rgb": [0.1, 0.2, 1.3],
        "model.point_sprites.type": "-X",
    }
    options.update(d)
    for k, v in d.items():
        assert options[k] == v, f"{k} was not set correctly"


def test_update_from_kv_pairs():
    options = f3d.Options()

    d: dict[str, Any] = {
        "ui.axis": True,
        "model.material.roughness": 0.8,
        "scene.animation.speed_factor": 3.8,
        "render.raytracing.samples": 8,
        "model.color.rgb": [0.1, 0.2, 1.3],
        "model.point_sprites.type": "-X",
    }
    options.update(d.items())
    for k, v in d.items():
        assert options[k] == v, f"{k} was not set correctly"


def test_update_from_invalid_kv_pairs():
    options = f3d.Options()

    items: Any = (
        ("ui.axis", True),
        ("model.material.roughness", 0.8),
        ("a", "b", 3),
    )
    with pytest.raises(ValueError):
        options.update(items)


def test_is_same():
    options1 = f3d.Options()
    options2 = f3d.Options()
    options1["ui.axis"] = True
    options2["ui.axis"] = False
    assert not options2.is_same(options1, "ui.axis")


def test_is_copy():
    options1 = f3d.Options()
    options2 = f3d.Options()
    options1["ui.axis"] = True
    options2["ui.axis"] = False
    assert not options2.is_same(options1, "ui.axis")
    options2.copy(options1, "ui.axis")
    assert options2.is_same(options1, "ui.axis")

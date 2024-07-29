import pytest

import f3d


def test_closest_option():
    option, distance = f3d.Options().get_closest_option("scene_direction")
    assert option == "scene.up_direction"
    assert distance == 3


def test_setitem():
    options = f3d.Options()
    options["interactor.axis"] = "false"
    options["model.material.roughness"] = "0.3"
    options["render.raytracing.samples"] = "5"
    options["model.color.rgb"] = "1, 1, 1"
    options["scene.up_direction"] = "+Y"


def test_getitem():
    engine = f3d.Engine(f3d.Window.NONE)
    options = engine.options
    assert options["interactor.axis"] == "false"
    assert options["model.material.roughness"] == "0.3"
    assert options["render.raytracing.samples"] == "5"
    assert options["model.color.rgb"] == "1, 1, 1"
    assert options["scene.up_direction"] == "+Y"


# TODO improve when bindings is finalized
# def test_get_non_existent_key():
#    options = f3d.Options()
#    with pytest.raises(KeyError):
#        assert options["hello"] == "world"


# def test_set_non_existent_key():
#    options = f3d.Options()
#    with pytest.raises(KeyError):
#        options["hello"] = "world"


def test_len():
    options = f3d.Options()
    assert len(options) == len(options.keys())


def test_iter():
    options = f3d.Options()
    assert list(options) == options.keys()


def test_contains():
    options = f3d.Options()
    assert "model.color.rgb" in options
    assert "hello.world" not in options


def test_set_options():
    options = f3d.Options()
    options["interactor.axis"] = "true"
    options["model.material.roughness"] = "0.7"
    options["render.raytracing.samples"] = "2"
    options["model.color.rgb"] = "0, 1, 1"
    options["scene.up_direction"] = "-Z"

    engine = f3d.Engine(f3d.Window.NONE)
    engine.options = options
    assert engine.options["interactor.axis"] == "true"
    assert engine.options["model.material.roughness"] == "0.7"
    assert engine.options["render.raytracing.samples"] == "2"
    assert engine.options["model.color.rgb"] == "0, 1, 1"
    assert engine.options["scene.up_direction"] == "-Z"


def test_to_dict():
    options = f3d.Options()
    options_dict = dict(options)

    assert len(options) > 0
    assert len(options) == len(options_dict)
    for k, v in options_dict.items():
        assert options[k] == v


def test_update_from_dict():
    options = f3d.Options()

    d = {
        "interactor.axis": "true",
        "model.material.roughness": "0.8",
        "render.raytracing.samples": "8",
        "model.color.rgb": "0.1, 0.2, 1.3",
        "scene.up_direction": "-X",
    }
    options.update(d)
    for k, v in d.items():
        assert options[k] == v, f"{k} was not set correctly"


def test_update_from_kv_pairs():
    options = f3d.Options()

    d = {
        "interactor.axis": "true",
        "model.material.roughness": "0.8",
        "render.raytracing.samples": "8",
        "model.color.rgb": "0.1, 0.2, 1.3",
        "scene.up_direction": "-X",
    }
    options.update(d.items())
    for k, v in d.items():
        assert options[k] == v, f"{k} was not set correctly"


def test_is_same():
    options1 = f3d.Options()
    options2 = f3d.Options()
    options1["interactor.axis"] = "true"
    options2["interactor.axis"] = "false"
    assert not options2.is_same(options1, "interactor.axis")


def test_is_copy():
    options1 = f3d.Options()
    options2 = f3d.Options()
    options1["interactor.axis"] = "true"
    options2["interactor.axis"] = "false"
    assert not options2.is_same(options1, "interactor.axis")
    options2.copy(options1, "interactor.axis")
    assert options2.is_same(options1, "interactor.axis")

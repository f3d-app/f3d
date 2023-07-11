import os

import pytest

import f3d
import f3d_init


def test_closest_option():
    option, distance = f3d.options().get_closest_option("scene-direction")
    assert option == "scene.up-direction"
    assert distance == 3


def test_setitem():
    options = f3d.options()
    options["interactor.axis"] = False
    options["model.material.roughness"] = 0.3
    options["render.raytracing.samples"] = 5
    options["model.color.rgb"] = [1.0, 1.0, 1.0]
    options["scene.up-direction"] = "+Y"


def test_getitem():
    engine = f3d.engine(f3d.window.NONE)
    options = engine.options

    assert options["interactor.axis"] is False
    assert options["model.material.roughness"] == 0.3
    assert options["render.raytracing.samples"] == 5
    assert options["model.color.rgb"] == [1.0, 1.0, 1.0]
    assert options["scene.up-direction"] == "+Y"


def test_get_non_existent_key():
    options = f3d.options()
    with pytest.raises(KeyError):
        assert options["hello"] == "world"

@pytest.mark.xfail(reason="f3d::options::set does not throw, should it?")
def test_set_non_existent_key():
    options = f3d.options()
    with pytest.raises(KeyError):
        options["hello"] = "world"


def test_len():
    options = f3d.options()
    assert len(options) == len(options.keys())


def test_iter():
    options = f3d.options()
    assert list(options) == options.keys()


def test_contains():
    options = f3d.options()
    assert "model.color.rgb" in options
    assert "hello.world" not in options


def test_set_options():
    options = f3d.options()
    options["interactor.axis"] = True
    options["model.material.roughness"] = 0.7
    options["render.raytracing.samples"] = 2
    options["model.color.rgb"] = [0.0, 1.0, 1.0]
    options["scene.up-direction"] = "-Z"

    engine = f3d.engine(f3d.window.NONE)
    engine.options = options
    assert engine.options["interactor.axis"] is True
    assert engine.options["model.material.roughness"] == 0.7
    assert engine.options["render.raytracing.samples"] == 2
    assert engine.options["model.color.rgb"] == [0.0, 1.0, 1.0]
    assert engine.options["scene.up-direction"] == "-Z"


def test_to_dict():
    options = f3d.options()
    options_dict = dict(options)

    assert len(options) > 0
    assert len(options) == len(options_dict)
    for k, v in options_dict.items():
        assert options[k] == v


def test_update_from_dict():
    options = f3d.options()

    d = {
        "interactor.axis": True,
        "model.material.roughness": 0.8,
        "render.raytracing.samples": 8,
        "model.color.rgb": [0.1, 0.2, 1.3],
        "scene.up-direction": "-X",
    }
    options.update(d)
    for k, v in d.items():
        assert options[k] == v, f"{k} was not set correctly"


def test_update_from_kv_pairs():
    options = f3d.options()

    d = {
        "interactor.axis": True,
        "model.material.roughness": 0.8,
        "render.raytracing.samples": 8,
        "model.color.rgb": [0.1, 0.2, 1.3],
        "scene.up-direction": "-X",
    }
    options.update(d.items())
    for k, v in d.items():
        assert options[k] == v, f"{k} was not set correctly"


def test_is_same():
    options1 = f3d.options()
    options2 = f3d.options()
    options1["interactor.axis"] = True
    options2["interactor.axis"] = False
    assert not options2.is_same(options1, "interactor.axis")


def test_is_copy():
    options1 = f3d.options()
    options2 = f3d.options()
    options1["interactor.axis"] = True
    options2["interactor.axis"] = False
    assert not options2.is_same(options1, "interactor.axis")
    options2.copy(options1, "interactor.axis")
    assert options2.is_same(options1, "interactor.axis")


@pytest.mark.skipif(
    os.environ.get("F3D_NO_DEPRECATED"), reason="F3D_NO_DEPRECATED is set"
)
def test_legacy_tests():
    engine = f3d.engine(f3d.window.NONE)

    assert engine.getOptions().getAsBool("interactor.axis") is False
    assert engine.getOptions().getAsDouble("model.material.roughness") == 0.3
    assert engine.getOptions().getAsInt("render.raytracing.samples") == 5
    assert engine.getOptions().getAsDoubleVector("model.color.rgb") == [1.0, 1.0, 1.0]
    assert engine.getOptions().getAsString("scene.up-direction") == "+Y"

    closest = engine.getOptions().getClosestOption("scene-direction")
    assert closest[0] == "scene.up-direction"
    assert closest[1] == 3

    options = f3d.options()
    options.set("interactor.axis", True)
    options.set("model.material.roughness", 0.7)
    options.set("render.raytracing.samples", 2)
    options.set("model.color.rgb", [0.0, 1.0, 1.0])
    options.set("scene.up-direction", "-Z")

    engine.setOptions(options)

    assert engine.getOptions().getAsBool("interactor.axis") is True
    assert engine.getOptions().getAsDouble("model.material.roughness") == 0.7
    assert engine.getOptions().getAsInt("render.raytracing.samples") == 2
    assert engine.getOptions().getAsDoubleVector("model.color.rgb") == [0.0, 1.0, 1.0]
    assert engine.getOptions().getAsString("scene.up-direction") == "-Z"

    assert len(options.getNames()) > 0

    options2 = engine.getOptions()
    options2.set("interactor.axis", False)
    assert not options2.isSame(options, "interactor.axis")
    options2.copy(options, "interactor.axis")
    assert options2.isSame(options, "interactor.axis")

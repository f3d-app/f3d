from pathlib import Path
import os
import pytest
import tempfile

import f3d


def test_legacy_Camera():
    engine = f3d.engine(f3d.window.NATIVE_OFFSCREEN)
    camera = engine.getWindow().getCamera()

    # Behavior is tested in the SDK test, test only the bindings

    pos = 1, 2, 3
    foc = 1, 22, 3
    up = 0, 0, 1
    angle = 32

    camera.setPosition(pos)
    camera.setFocalPoint(foc)
    camera.setViewUp(up)
    camera.setViewAngle(angle)
    assert camera.getPosition() == pos
    assert camera.getFocalPoint() == foc
    assert camera.getViewUp() == up
    assert camera.getViewAngle() == angle

    state = camera.getState()
    assert state.pos == pos
    assert state.foc == foc
    assert state.up == up
    assert state.angle == angle

    new_default_state = f3d.camera_state_t()
    assert new_default_state.pos == (0, 0, 1)
    assert new_default_state.foc == (0, 0, 0)
    assert new_default_state.up == (0, 1, 0)
    assert new_default_state.angle == 30

    camera.setState(new_default_state)
    assert camera.getPosition() == new_default_state.pos
    assert camera.getFocalPoint() == new_default_state.foc
    assert camera.getViewUp() == new_default_state.up
    assert camera.getViewAngle() == new_default_state.angle

    new_state = f3d.camera_state_t(pos, foc, up, angle)
    assert new_state.pos == pos
    assert new_state.foc == foc
    assert new_state.up == up
    assert new_state.angle == angle

    camera.setState(new_state)
    assert camera.getPosition() == new_state.pos
    assert camera.getFocalPoint() == new_state.foc
    assert camera.getViewUp() == new_state.up
    assert camera.getViewAngle() == new_state.angle

    camera.dolly(10)
    angle = 30
    camera.roll(angle)
    camera.azimuth(angle)
    camera.yaw(angle)
    camera.elevation(angle)
    camera.pitch(angle)

    state = camera.getState()
    camera.getState(state)
    camera.setState(state)

    camera.setCurrentAsDefault()
    camera.resetToBounds()
    camera.resetToDefault()


def test_legacy_CompareWithFile():
    testingDir = str(Path(__file__).parent.parent.parent / "testing")
    dataset = testingDir + "/data/cow.vtp"
    reference = testingDir + "/baselines/TestPythonCompareWithFile.png"
    output = tempfile.gettempdir() + "/TestPythonCompareWithFile.png"
    outputDiff = tempfile.gettempdir() + "/TestPythonCompareWithFile.diff.png"

    f3d.engine.autoloadPlugins()

    engine = f3d.engine(f3d.window.NATIVE_OFFSCREEN)
    engine.getWindow().setSize(300, 300)

    # verify the size is properly set
    assert engine.getWindow().getWidth() == 300
    assert engine.getWindow().getHeight() == 300

    engine.getLoader().loadGeometry(dataset, True)

    img = engine.getWindow().renderToImage()
    img.save(output)

    diff = f3d.image()
    error = 0.0

    ret = img.compare(f3d.image(reference), 100, diff, error)

    if not ret:
        print("img compare failing with diff=" + error)
        diff.save(outputDiff)

    assert ret is True


def test_legacy_ImageData():
    engine = f3d.engine(f3d.window.NATIVE_OFFSCREEN)
    window = engine.getWindow()
    window.setSize(300, 200)

    """with background -> RGB image"""

    img = window.renderToImage()
    width = img.getWidth()
    height = img.getHeight()
    depth = img.getChannelCount()
    data = img.getContent()

    assert width == window.getWidth()
    assert height == window.getHeight()
    assert depth == 3
    assert isinstance(data, (bytes, bytearray))
    assert len(data) == depth * width * height

    """without background -> RGBA image"""

    img = window.renderToImage(True)
    width = img.getWidth()
    height = img.getHeight()
    depth = img.getChannelCount()
    data = img.getContent()

    assert width == window.getWidth()
    assert height == window.getHeight()
    assert depth == 4
    assert isinstance(data, (bytes, bytearray))
    assert len(data) == depth * width * height

    """set data back"""

    img.setData(data)
    assert img.getContent() == data

    """check channel type and save image"""

    assert img.getChannelType() == f3d.image.ChannelType.BYTE
    assert img.getChannelTypeSize() == 1

    fn = tempfile.gettempdir() + "/TestPythonSaveFile.bmp"
    img.save(fn, f3d.image.SaveFormat.BMP)
    assert os.path.isfile(fn)

    """attempt to set partial data back"""

    try:
        img.setData(data[:-1])
        assert False, "expected exception"
    except ValueError:
        assert True


def test_legacy_Options():
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


def test_legacy_Plugins():
    base_dir = os.path.dirname(f3d.__file__)
    plugins = f3d.engine.getPluginsList(base_dir + "/share/f3d/plugins")
    plugins += f3d.engine.getPluginsList(base_dir + "/../share/f3d/plugins")
    plugins += f3d.engine.getPluginsList(base_dir + "/../../share/f3d/plugins")

    assert len(plugins) > 0
    assert plugins.index("native") >= 0


def test_legacy_Utils():
    distance = f3d.utils.textDistance("modle", "model")

    assert distance == 2

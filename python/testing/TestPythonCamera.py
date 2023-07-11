import os

import pytest

import f3d
import f3d_init


def test_properties():
    engine = f3d.engine(f3d.window.NATIVE_OFFSCREEN)
    camera = engine.window.camera

    pos = 1, 2, 3
    foc = 1, 22, 3
    up = 0, 0, 1
    angle = 32

    camera.position = pos
    camera.focal_point = foc
    camera.view_up = up
    camera.view_angle = angle
    assert camera.position == pos
    assert camera.focal_point == foc
    assert camera.view_up == up
    assert camera.view_angle == angle


def test_get_state():
    engine = f3d.engine(f3d.window.NATIVE_OFFSCREEN)
    camera = engine.window.camera

    pos = 1, 2, 3
    foc = 1, 22, 3
    up = 0, 0, 1
    angle = 32

    camera.position = pos
    camera.focal_point = foc
    camera.view_up = up
    camera.view_angle = angle

    assert camera.state.pos == pos
    assert camera.state.foc == foc
    assert camera.state.up == up
    assert camera.state.angle == angle


def test_set_state():
    engine = f3d.engine(f3d.window.NATIVE_OFFSCREEN)
    camera = engine.window.camera

    state = f3d.camera_state_t((1, 2, 3), (1, 22, 3), (0, 0, 1), 32)
    camera.state = state

    assert camera.position == state.pos
    assert camera.focal_point == state.foc
    assert camera.view_up == state.up
    assert camera.view_angle == state.angle


def test_default_state():
    new_state = f3d.camera_state_t()
    assert new_state.pos == (0, 0, 1)
    assert new_state.foc == (0, 0, 0)
    assert new_state.up == (0, 1, 0)
    assert new_state.angle == 30


@pytest.mark.xfail(reason="camera_state_t equality not implemented")
def test_state_compare():

    state1 = f3d.camera_state_t((1, 2, 3), (1, 22, 3), (0, 0, 1), 32)
    state2 = f3d.camera_state_t((1, 2, 3), (1, 22, 3), (0, 0, 1), 32)
    assert state1 == state2

    state3 = f3d.camera_state_t((1, 2, 3), (1, 22, 3), (0, 0, 1), 25)
    assert state1 != state3


def test_moves():
    engine = f3d.engine(f3d.window.NATIVE_OFFSCREEN)
    camera = engine.window.camera

    camera.dolly(10)
    angle = 30
    camera.roll(angle)
    camera.azimuth(angle)
    camera.yaw(angle)
    camera.elevation(angle)
    camera.pitch(angle)


def test_resets():
    engine = f3d.engine(f3d.window.NATIVE_OFFSCREEN)
    camera = engine.window.camera
    camera.set_current_as_default()
    camera.reset_to_bounds()
    camera.reset_to_default()


@pytest.mark.skipif(
    os.environ.get("F3D_NO_DEPRECATED"), reason="F3D_NO_DEPRECATED is set"
)
def test_legacy_test():
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

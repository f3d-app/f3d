import pytest

import f3d


def test_properties():
    engine = f3d.Engine.create(True)
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
    engine = f3d.Engine.create(True)
    camera = engine.window.camera

    pos = 1, 2, 3
    foc = 1, 22, 3
    up = 0, 0, 1
    angle = 32

    camera.position = pos
    camera.focal_point = foc
    camera.view_up = up
    camera.view_angle = angle

    assert camera.state.position == pos
    assert camera.state.focal_point == foc
    assert camera.state.view_up == up
    assert camera.state.view_angle == angle


def test_set_state():
    engine = f3d.Engine.create(True)
    camera = engine.window.camera

    state = f3d.CameraState((1, 2, 3), (1, 22, 3), (0, 0, 1), 32)
    camera.state = state

    assert camera.position == state.position
    assert camera.focal_point == state.focal_point
    assert camera.view_up == state.view_up
    assert camera.view_angle == state.view_angle


def test_default_state():
    new_state = f3d.CameraState()
    assert new_state.position == (0, 0, 1)
    assert new_state.focal_point == (0, 0, 0)
    assert new_state.view_up == (0, 1, 0)
    assert new_state.view_angle == 30


@pytest.mark.xfail(reason="CameraState equality not implemented")
def test_state_compare():
    state1 = f3d.CameraState((1, 2, 3), (1, 22, 3), (0, 0, 1), 32)
    state2 = f3d.CameraState((1, 2, 3), (1, 22, 3), (0, 0, 1), 32)
    assert state1 == state2

    state3 = f3d.CameraState((1, 2, 3), (1, 22, 3), (0, 0, 1), 25)
    assert state1 != state3


def test_moves():
    engine = f3d.Engine.create(True)
    camera = engine.window.camera

    camera.dolly(10)
    angle = 30
    camera.roll(angle)
    camera.azimuth(angle)
    camera.yaw(angle)
    camera.elevation(angle)
    camera.pitch(angle)


def test_pan():
    engine = f3d.Engine.create(True)
    camera = engine.window.camera

    camera.state = f3d.CameraState((1, 2, 3), (1, 2, 13), (0, 1, 0), 40)
    camera.pan(1, 2)
    assert camera.state.position == (0, 4, 3)
    assert camera.state.focal_point == (0, 4, 13)

    camera.state = f3d.CameraState((1, 2, 3), (1, -2, 3), (0, 0, 1), 40)
    camera.pan(3, 4, 5)
    assert camera.state.position == (-2, -3, 7)
    assert camera.state.focal_point == (-2, -7, 7)


def test_resets():
    engine = f3d.Engine.create(True)
    camera = engine.window.camera
    camera.set_current_as_default()
    camera.reset_to_bounds()
    camera.reset_to_default()

import pytest

import f3d


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

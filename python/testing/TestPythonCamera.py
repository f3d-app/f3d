import sys
import os

if sys.platform.startswith("win32"):
    os.add_dll_directory(sys.argv[1])

import f3d

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

import sys
import os

if sys.platform.startswith("win32"):
    os.add_dll_directory(sys.argv[1])

import f3d

engine = f3d.engine(f3d.window.NATIVE_OFFSCREEN)
camera = engine.getWindow().getCamera()

# Behavior is tested in the SDK test, test only the bindings

# TODO Once angle_deg_t is an actual struct, this will need to be changed
angle = 30
camera.setViewAngle(angle)
camera.getViewAngle(angle)
angle = camera.getViewAngle()

# point3_t
xyz = 1, 2, 3
point = f3d.point3_t(*xyz)
assert tuple(point) == xyz
assert (point[0], point[1], point[2]) == xyz
point[1] += 1
assert tuple(point) == (xyz[0], xyz[1] + 1, xyz[2])

# vector3_t
vector = f3d.vector3_t(*xyz)
assert tuple(vector) == xyz
assert (vector[0], vector[1], vector[2]) == xyz
vector[1] += 1
assert tuple(vector) == (xyz[0], xyz[1] + 1, xyz[2])


# position as point3_t
point_in = f3d.point3_t(1, 2, 3)
point_out = f3d.point3_t(0, 0, 0)
camera.setPosition(point_in)
camera.getPosition(point_out)
assert tuple(point_out) == tuple(point_in)

# position as tuple
tuple_in = 3, 4, 5
camera.setPosition(tuple_in)
assert tuple(camera.getPosition()) == tuple_in


# focalPoint as point3_t
point_in = f3d.point3_t(1, 2, 3)
point_out = f3d.point3_t(0, 0, 0)
camera.setFocalPoint(point_in)
camera.getFocalPoint(point_out)
assert tuple(point_out) == tuple(point_in)

# focalPoint as tuple
tuple_in = 3, 4, 5
camera.setFocalPoint(tuple_in)
assert tuple(camera.getFocalPoint()) == tuple_in


# viewUp as vector3_t
camera.setPosition((1, 0, 0))
camera.setFocalPoint((0, 0, 0))
camera.setViewUp((0, 1, 0))

vector_in = f3d.vector3_t(0, 0, 1)
vector_out = f3d.vector3_t(0, 0, 0)
camera.setViewUp(vector_in)
camera.getViewUp(vector_out)
assert tuple(vector_out) == tuple(vector_in)

# viewUp as tuple
camera.setPosition((1, 0, 0))
camera.setFocalPoint((0, 0, 0))
camera.setViewUp((0, 1, 0))

tuple_in = 0, 0, 1
camera.setViewUp(tuple_in)
assert tuple(camera.getViewUp()) == tuple_in


camera.dolly(10)
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

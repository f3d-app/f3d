import f3d

engine = f3d.engine(f3d.window.NATIVE_OFFSCREEN)
camera = engine.getWindow().getCamera()

# Behavior is tested in the SDK test, test only the bindings

# TODO Once angle_deg_t is an actual struct, this will need to be changed
angle = 30
camera.setViewAngle(angle)
camera.getViewAngle(angle)
angle = camera.getViewAngle()

point = f3d.point3_t(0, 0, 0)
camera.setPosition(point)
camera.getPosition(point)
point = camera.getPosition()

camera.setFocalPoint(point)
camera.getFocalPoint(point)
point = camera.getFocalPoint()

vector = f3d.vector3_t(0, 0, 1)
camera.setViewUp(vector)
camera.getViewUp(vector)
vector = camera.getViewUp()

camera.dolly(10)
camera.roll(angle)
camera.azimuth(angle)
camera.yaw(angle)
camera.elevation(angle)
camera.pitch(angle)

camera.setCurrentAsDefault()
camera.resetToBounds()
camera.resetToDefault()

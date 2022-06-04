import f3d

engine = f3d.engine(f3d.window.NO_RENDER)

assert engine.getOptions().getAsBool("axis") is False
assert engine.getOptions().getAsDouble("roughness") == 0.3
assert engine.getOptions().getAsInt("samples") == 5
assert engine.getOptions().getAsDoubleVector("color") == [ 1., 1., 1.]
assert engine.getOptions().getAsIntVector("resolution") == [ 1000, 600 ]
assert engine.getOptions().getAsString("up") == "+Y"

options = f3d.options()
options.set("axis", True)
options.set("roughness", 0.7)
options.set("samples", 2)
options.set("color", [ 0., 1., 1.])
options.set("resolution", [ 300, 300 ])
options.set("up", "-Z")

engine.setOptions(options)

assert engine.getOptions().getAsBool("axis") is True
assert engine.getOptions().getAsDouble("roughness") == 0.7
assert engine.getOptions().getAsInt("samples") == 2
assert engine.getOptions().getAsDoubleVector("color") == [ 0., 1., 1.]
assert engine.getOptions().getAsIntVector("resolution") == [ 300, 300 ]
assert engine.getOptions().getAsString("up") == "-Z"

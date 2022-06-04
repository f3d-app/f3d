import f3d
import sys

dataset = sys.argv[1] + "data/cow.vtp"
reference = sys.argv[1] + "baselines/TestPythonCompareWithFile.png"
output = sys.argv[2] + "TestPythonCompareWithFile.png"
outputDiff = sys.argv[2] + "TestPythonCompareWithFile.diff.png"

engine = f3d.engine(f3d.window.NATIVE_OFFSCREEN)
engine.getOptions().set("initial-resolution", [ 600, 600 ])
engine.getLoader().addFile(dataset)
engine.getLoader().loadFile(f3d.loader.LoadFileEnum.LOAD_CURRENT)

engine.getWindow().render()

# check that the next load takes into consideration the resolution change
engine.getOptions().set("initial-resolution", [ 300, 300 ])
engine.getLoader().loadFile(f3d.loader.LoadFileEnum.LOAD_CURRENT)

img = engine.getWindow().renderToImage()
img.save(output)

diff = f3d.image()
error = 0.0

ret = img.compare(f3d.image(reference), 50, diff, error)

if not ret:
  diff.save(outputDiff)
  
assert ret is True

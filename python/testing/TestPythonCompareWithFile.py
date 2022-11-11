import f3d
import sys

dataset = sys.argv[1] + "data/cow.vtp"
reference = sys.argv[1] + "baselines/TestPythonCompareWithFile.png"
output = sys.argv[2] + "TestPythonCompareWithFile.png"
outputDiff = sys.argv[2] + "TestPythonCompareWithFile.diff.png"

f3d.engine.autoloadPlugins()

engine = f3d.engine(f3d.window.NATIVE_OFFSCREEN)
engine.getWindow().setSize(300, 300)

# verify the size is properly set
assert engine.getWindow().getWidth() == 300
assert engine.getWindow().getHeight() == 300

engine.getLoader().addFile(dataset).loadFile()

img = engine.getWindow().renderToImage()
img.save(output)

diff = f3d.image()
error = 0.0

ret = img.compare(f3d.image(reference), 50, diff, error)

if not ret:
  diff.save(outputDiff)
  
assert ret is True

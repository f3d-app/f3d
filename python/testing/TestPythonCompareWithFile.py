import sys
import f3d

dataset = sys.argv[1] + "/testing/data/cow.vtp"
reference = sys.argv[1] + "/testing/baselines/TestPythonCompareWithFile.png"
output = sys.argv[2] + "/Testing/Temporary/TestPythonCompareWithFile.png"
outputDiff = sys.argv[2] + "/Testing/Temporary/TestPythonCompareWithFile.diff.png"

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

ret = img.compare(f3d.image(reference), 50, diff, error)

if not ret:
    diff.save(outputDiff)

assert ret is True

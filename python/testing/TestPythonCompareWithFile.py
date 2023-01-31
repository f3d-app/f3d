import sys
if sys.platform.startswith('win32'):
  os.add_dll_directory(sys.argv[1])

import f3d

dataset = sys.argv[2] + "data/cow.vtp"
reference = sys.argv[2] + "baselines/TestPythonCompareWithFile.png"
output = sys.argv[3] + "TestPythonCompareWithFile.png"
outputDiff = sys.argv[3] + "TestPythonCompareWithFile.diff.png"

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

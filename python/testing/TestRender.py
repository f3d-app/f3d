import f3d
import sys

dataset = sys.argv[1] + "data/cow.vtp"
reference = sys.argv[1] + "baselines/TestSDKCompareWithFile.png"
output = sys.argv[2] + "TestPythonCompareWithFile.png"
outputDiff = sys.argv[2] + "TestPythonCompareWithFile.diff.png"

engine = f3d.engine(f3d.engine.CREATE_WINDOW | f3d.engine.WINDOW_OFFSCREEN)
engine.getOptions().set("resolution", [ 600, 600 ])
engine.getLoader().addFile(dataset)
engine.getLoader().loadFile(f3d.loader.LoadFileEnum.LOAD_CURRENT)

engine.getWindow().render()

# check that the next render takes into consideration the resolution change
engine.getOptions().set("resolution", [ 300, 300 ])
engine.getWindow().update()

img = engine.getWindow().renderToImage()
img.save(output)

diff = f3d.image()

assert img.compare(f3d.image(reference), diff, 50) is True

diff.save(outputDiff)

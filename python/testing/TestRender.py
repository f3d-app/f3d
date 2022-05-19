import f3d
import sys

dataset = sys.argv[1] + "data/cow.vtp"
reference = sys.argv[1] + "baselines/TestSDKCompareWithFile.png"
output = sys.argv[2] + "TestSDKCompareWithFile.png"

engine = f3d.engine(f3d.engine.CREATE_WINDOW | f3d.engine.WINDOW_OFFSCREEN)
engine.getOptions().set("resolution", [ 600, 600 ])
engine.getLoader().addFile(dataset)
engine.getLoader().loadFile(f3d.loader.LoadFileEnum.LOAD_CURRENT)

engine.getWindow().render()

# check that the next render takes into consideration the resolution change
engine.getOptions().set("resolution", [ 300, 300 ])
engine.getWindow().update()

assert engine.getWindow().renderAndCompareWithFile(reference, 50, False, output) is True

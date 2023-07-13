import os
import f3d

engine = f3d.engine(f3d.window.NATIVE_OFFSCREEN)
window = engine.getWindow()
window.setSize(300, 200)


'''with background -> RGB image'''

img = window.renderToImage()
width = img.getWidth()
height = img.getHeight()
depth = img.getChannelCount()
data = img.getContent()

assert width == window.getWidth()
assert height == window.getHeight()
assert depth == 3
assert isinstance(data, (bytes, bytearray))
assert len(data) == depth * width * height


'''without background -> RGBA image'''

img = window.renderToImage(True)
width = img.getWidth()
height = img.getHeight()
depth = img.getChannelCount()
data = img.getContent()

assert width == window.getWidth()
assert height == window.getHeight()
assert depth == 4
assert isinstance(data, (bytes, bytearray))
assert len(data) == depth * width * height


'''set data back'''

img.setContent(data)
assert img.getContent() == data

'''check channel type and save image'''

assert img.getChannelType() == f3d.image.ChannelType.BYTE
assert img.getChannelTypeSize() == 1

img.save(sys.argv[2] + "/Testing/Temporary/TestPythonSaveFile.bmp", f3d.image.SaveFormat.BMP)
assert os.path.isfile(sys.argv[2] + "/Testing/Temporary/TestPythonSaveFile.bmp")


'''attempt to set partial data back'''

try:
    img.setContent(data[:-1])
    assert False, 'expected exception'
except ValueError:
    assert True

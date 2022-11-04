import f3d

engine = f3d.engine(f3d.window.NATIVE_OFFSCREEN)
window = engine.getWindow()
window.setSize(300, 200)


'''with background -> RGB image'''

img = window.renderToImage()
width = img.getWidth()
height = img.getHeight()
depth = img.getChannelCount()
data = img.getData()

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
data = img.getData()

assert width == window.getWidth()
assert height == window.getHeight()
assert depth == 4
assert isinstance(data, (bytes, bytearray))
assert len(data) == depth * width * height


'''set data back'''

img.setData(data)
assert img.getData() == data


'''attempt to set partial data back'''

try:
    img.setData(data[:-1])
    assert False, 'expected exception'
except ValueError:
    assert True

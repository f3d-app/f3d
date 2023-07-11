import os
import sys
if sys.platform.startswith('win32'):
  os.add_dll_directory(sys.argv[1])

import f3d

from pathlib import Path
from tempfile import TemporaryDirectory

def test_image(out_dir = None):
  if out_dir is None:
    tmp_dir = TemporaryDirectory()
    out_dir = Path(tmp_dir.name)

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

  img.save(str(out_dir / "TestPythonSaveFile.bmp"), f3d.image.SaveFormat.BMP)
  assert os.path.isfile(str(out_dir / "TestPythonSaveFile.bmp"))


  '''attempt to set partial data back'''

  try:
      img.setContent(data[:-1])
      assert False, 'expected exception'
  except ValueError:
      assert True

if __name__ == '__main__':
  test_image(Path(sys.argv[3]) / "Testing/Temporary")

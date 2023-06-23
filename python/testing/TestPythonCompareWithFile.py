import sys
if sys.platform.startswith('win32'):
  import os
  os.add_dll_directory(sys.argv[1])

import f3d

from pathlib import Path
from tempfile import TemporaryDirectory

TESTS_DATA_DIR = Path(__file__).parent.parent.parent

def test_compare(in_dir = TESTS_DATA_DIR, out_dir = None):
  if out_dir is None:
    tmp_dir = TemporaryDirectory()
    out_dir = Path(tmp_dir.name)
    
  dataset = str(in_dir / "testing" / "data" / "cow.vtp")
  reference = str(in_dir / "testing" / "baselines" / "TestPythonCompareWithFile.png")
  output = str(out_dir / "TestPythonCompareWithFile.png")
  outputDiff = str(out_dir / "TestPythonCompareWithFile.diff.png")

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

if __name__ == '__main__':
  test_compare(Path(sys.argv[2]), Path(sys.argv[3]))

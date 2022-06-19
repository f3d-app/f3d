import f3d
import sys
from pathlib import Path
from tempfile import TemporaryDirectory

TESTS_DATA_DIR = Path(__file__).parent.parent.parent / "testing"

def test_compare(in_dir = TESTS_DATA_DIR, out_dir = None):
  if out_dir is None:
    tmp_dir = TemporaryDirectory()
    out_dir = Path(tmp_dir.name)
    
  dataset = str(in_dir / "data" / "cow.vtp")
  reference = str(in_dir / "baselines" / "TestPythonCompareWithFile.png")
  output = str(out_dir / "TestPythonCompareWithFile.png")
  outputDiff = str(out_dir / "TestPythonCompareWithFile.diff.png")

  engine = f3d.engine(f3d.window.NATIVE_OFFSCREEN)
  engine.getWindow().setSize(300, 300);
  engine.getLoader().addFile(dataset)
  engine.getLoader().loadFile(f3d.loader.LoadFileEnum.LOAD_CURRENT)

  img = engine.getWindow().renderToImage()
  img.save(output)

  diff = f3d.image()
  error = 0.0

  ret = img.compare(f3d.image(reference), 50, diff, error)

  if not ret:
    diff.save(outputDiff)
    
  assert ret is True

if __name__ == '__main__':
  test_compare(Path(sys.argv[1]), Path(sys.argv[2]))
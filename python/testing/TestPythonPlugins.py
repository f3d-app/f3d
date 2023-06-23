import sys
import os

if sys.platform.startswith('win32'):
  os.add_dll_directory(sys.argv[1])

import f3d

from pathlib import Path

ROOT_DIR = Path(f3d.__file__).parent

def test_plugins(root_dir = ROOT_DIR):
  plugins = f3d.engine.getPluginsList(str(root_dir / 'share' / 'f3d' / 'plugins'))
  assert len(plugins) > 0
  assert plugins.index('native') >= 0

if __name__ == '__main__':
  test_plugins(Path(f3d.__file__).parent.parent)

import sys
import os

if sys.platform.startswith('win32'):
  os.add_dll_directory(sys.argv[1])

import f3d

def test_plugins():
  plugins = f3d.engine.getPluginsList(os.path.dirname(f3d.__file__) + '/share/f3d/plugins')
  assert len(plugins) > 0
  assert plugins.index('native') >= 0

if __name__ == '__main__':
  test_plugins()

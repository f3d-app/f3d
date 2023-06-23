import sys
if sys.platform.startswith('win32'):
  import os
  os.add_dll_directory(sys.argv[1])

import f3d

def test_plugins():
  plugins = f3d.engine.getPluginsList(sys.argv[3] + '/share/f3d/plugins')
  assert len(plugins) > 0
  assert plugins.index('native') >= 0

if __name__ == '__main__':
  test_plugins()

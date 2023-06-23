import sys
if sys.platform.startswith('win32'):
  import os
  os.add_dll_directory(sys.argv[1])

import f3d

distance = f3d.utils.textDistance('modle', 'model')

assert distance == 2

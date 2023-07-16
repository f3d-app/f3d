import sys
import f3d

plugins = f3d.engine.getPluginsList(sys.argv[2] + "/share/f3d/plugins")
assert len(plugins) > 0
assert plugins.index("native") >= 0

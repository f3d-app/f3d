import os

import pytest

import f3d


def test_plugins_list():
    base_dir = os.path.dirname(f3d.__file__)
    plugins = f3d.Engine.get_plugins_list(base_dir + "/share/f3d/plugins")
    plugins += f3d.Engine.get_plugins_list(base_dir + "/../share/f3d/plugins")
    plugins += f3d.Engine.get_plugins_list(base_dir + "/../../share/f3d/plugins")

    assert len(plugins) > 0
    assert plugins.index("native") >= 0

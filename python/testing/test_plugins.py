import os

import pytest

import f3d


def test_plugins_list():
    base_dir = os.path.dirname(f3d.__file__)
    plugins = f3d.engine.get_plugins_list(base_dir + "/share/f3d/plugins")

    # in case we are testing directly in the build with a multi-config generator
    plugins += f3d.engine.get_plugins_list(base_dir + "/../share/f3d/plugins")

    assert len(plugins) > 0
    assert plugins.index("native") >= 0

import os

import pytest

import f3d


def test_plugins_list():
    print(os.path.dirname(f3d.__file__))
    plugins = f3d.engine.get_plugins_list(os.path.dirname(f3d.__file__) + "/share/f3d/plugins")
    assert len(plugins) > 0
    assert plugins.index("native") >= 0

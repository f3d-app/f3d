import os

import pytest

import f3d


@pytest.fixture
def cmake_binary_dir(pytestconfig):
    return pytestconfig.getoption("cmake_binary_dir")


def test_plugins_list(cmake_binary_dir):
    plugins = f3d.engine.get_plugins_list(cmake_binary_dir + "/share/f3d/plugins")
    assert len(plugins) > 0
    assert plugins.index("native") >= 0

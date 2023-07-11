import os

import pytest

import f3d
import f3d_init


@pytest.fixture
def cmake_binary_dir(pytestconfig):
    return pytestconfig.getoption("cmake_binary_dir")


@pytest.mark.skipif(
    os.environ.get("F3D_NO_DEPRECATED"), reason="F3D_NO_DEPRECATED is set"
)
def test_legacy_tests(cmake_binary_dir):
    plugins = f3d.engine.getPluginsList(cmake_binary_dir + "/share/f3d/plugins")
    assert len(plugins) > 0
    assert plugins.index("native") >= 0

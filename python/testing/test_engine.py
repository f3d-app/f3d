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


def test_get_lib_info():
    lib_info = f3d.engine.get_lib_info()

    assert isinstance(lib_info, f3d.LibInformation)

    assert isinstance(lib_info.version, str) and lib_info.version
    assert isinstance(lib_info.version_full, str) and lib_info.version_full
    assert isinstance(lib_info.build_date, str) and lib_info.build_date
    assert isinstance(lib_info.build_system, str) and lib_info.build_system
    assert isinstance(lib_info.compiler, str) and lib_info.compiler

    assert isinstance(lib_info.modules, dict) and lib_info.modules
    for key, value in lib_info.modules.items():
        assert isinstance(key, str) and key
        assert isinstance(value, bool)

    assert isinstance(lib_info.vtk_version, str) and lib_info.vtk_version

    assert isinstance(lib_info.copyrights, list) and lib_info.copyrights
    for copyright in lib_info.copyrights:
        assert isinstance(copyright, str) and copyright

    assert isinstance(lib_info.license, str) and lib_info.license


def test_get_readers_info():
    readers_info = f3d.engine.get_readers_info()

    assert isinstance(readers_info, list) and readers_info

    for reader in readers_info:
        assert isinstance(reader, f3d.ReaderInformation)

        assert isinstance(reader.name, str) and reader.name
        assert isinstance(reader.description, str) and reader.description

        assert isinstance(reader.extensions, list) and reader.extensions
        for ext in reader.extensions:
            assert isinstance(ext, str) and ext

        assert isinstance(reader.mime_types, list) and reader.mime_types
        for mime in reader.mime_types:
            assert isinstance(mime, str) and mime

        assert isinstance(reader.plugin_name, str) and reader.plugin_name
        assert isinstance(reader.has_scene_reader, bool)
        assert isinstance(reader.has_geometry_reader, bool)

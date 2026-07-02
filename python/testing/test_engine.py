from pathlib import Path

import pytest

import f3d


def test_plugins_list():
    base_dir = Path(f3d.__file__).parent
    plugins = f3d.Engine.get_plugins_list(base_dir / "share/f3d/plugins")
    plugins += f3d.Engine.get_plugins_list(base_dir / "../share/f3d/plugins")
    plugins += f3d.Engine.get_plugins_list(base_dir / "../../share/f3d/plugins")

    assert len(plugins) > 0
    assert plugins.index("native") >= 0


def test_get_lib_info():
    lib_info = f3d.Engine.get_lib_info()

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
    readers_info = f3d.Engine.get_readers_info()

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


def test_get_rendering_backend_list():
    backends = f3d.Engine.get_rendering_backend_list()

    assert isinstance(backends, dict) and len(backends) == 5


def test_reader_options():
    reader_option_names = f3d.Engine.get_all_reader_option_names()
    assert reader_option_names is not None

    with pytest.raises(KeyError):
        f3d.Engine.set_reader_option("inexistent", "value")


def test_statefile(tmp_path):
    testing_dir = Path(__file__).parent.parent.parent / "testing"
    cow = testing_dir / "data/cow.vtp"
    statefile = tmp_path / "state.json"

    f3d.Engine.autoload_plugins()

    src = f3d.Engine.create_none()
    src.options["render.background.color"] = [1.0, 0.0, 0.0]
    src.options["ui.scalar_bar"] = True
    src.scene.add(cow)

    src.dump().to_file(statefile)
    assert statefile.exists()

    # File based round-trip
    dst = f3d.Engine.create_none()
    dst.load(f3d.Engine.State.from_file(statefile))
    assert dst.options["render.background.color"] == [1.0, 0.0, 0.0]
    assert dst.options["ui.scalar_bar"] is True
    assert len(dst.scene.get_added_files()) == 1
    assert Path(dst.scene.get_added_files()[0]) == cow


def test_statefile_string():
    testing_dir = Path(__file__).parent.parent.parent / "testing"
    cow = testing_dir / "data/cow.vtp"

    f3d.Engine.autoload_plugins()

    src = f3d.Engine.create_none()
    src.options["render.background.color"] = [1.0, 0.0, 0.0]
    src.scene.add(cow)

    content = src.dump().to_string()
    dst = f3d.Engine.create_none()
    dst.load(f3d.Engine.State.from_string(content))
    assert dst.options["render.background.color"] == [1.0, 0.0, 0.0]
    assert len(dst.scene.get_added_files()) == 1


def test_statefile_invalid():
    with pytest.raises(RuntimeError):
        f3d.Engine.State.from_file("/does/not/exist/state.json")
    with pytest.raises(RuntimeError):
        f3d.Engine.State.from_string("{ not valid json")

import f3d


def test_get_lib_info():
    lib_info = f3d.engine.get_lib_info()

    assert isinstance(lib_info, f3d.LibInformation)

    assert isinstance(lib_info.version, str)
    assert isinstance(lib_info.version_full, str)
    assert isinstance(lib_info.build_date, str)
    assert isinstance(lib_info.build_system, str)
    assert isinstance(lib_info.compiler, str)

    assert isinstance(lib_info.modules, dict)
    for key, value in lib_info.modules.items():
        assert isinstance(key, str)
        assert isinstance(value, bool)

    assert isinstance(lib_info.vtk_version, str)

    assert isinstance(lib_info.copyrights, list)
    for copyright in lib_info.copyrights:
        assert isinstance(copyright, str)

    assert isinstance(lib_info.license, str)

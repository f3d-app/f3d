import f3d


def test_get_readers_info():
    readers_info = f3d.engine.get_readers_info()

    assert isinstance(readers_info, list)

    for reader in readers_info:
        assert isinstance(reader, f3d.ReaderInformation)

        assert isinstance(reader.name, str)
        assert isinstance(reader.description, str)

        assert isinstance(reader.extensions, list)
        for ext in reader.extensions:
            assert isinstance(ext, str)

        assert isinstance(reader.mime_types, list)
        for mime in reader.mime_types:
            assert isinstance(mime, str)

        assert isinstance(reader.plugin_name, str)
        assert isinstance(reader.has_scene_reader, bool)
        assert isinstance(reader.has_geometry_reader, bool)

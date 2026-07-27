# Plugins

F3D officially supports plugins for certain file formats. If you installed F3D using the binary
release, you do not need to load these plugins manually when opening your file.
F3D loads them automatically.

However, if you installed F3D using a package manager,
the packager may bundle the plugins into different packages or
list plugin dependencies as optional to reduce the number of dependencies of the main package.

To open a file that requires a plugin, first install all necessary
dependencies. Then load the plugin with the `--load-plugins=<path or name>`
[command-line option](03-OPTIONS.md).

Alternatively, you can add your plugin directly in the
[configuration file](06-CONFIGURATION_FILE.md) if it isn't there already. You can specify one or
multiple plugins in a single comma-separated list, like in the example below:

```
[
  {
    "match": ".*(file_extension)",
    "options": {
      "load-plugins": "plugin1,plugin2"
    }
  }
]
```

## Supported plugins

F3D supports loading plugins and contains the following plugins:
`native`, `alembic`, `assimp`, `draco`, `hdf`, `occt`, `pdal`, `usd`, `vdb`, `webifc`.

See [the format list](02-SUPPORTED_FORMATS.md) for more details.

> [!NOTE]
> The `native` plugin is always loaded automatically.
> If you downloaded the binaries from the release page, you do not need to specify the plugins above manually. F3D loads them automatically.

### Plugin availability per binary

Not every binary ships every plugin, so a file may open in one binary but not another. This table shows which plugins are available in each official binary. The `native` plugin is always present.

| Plugin    | Desktop (installer) | Python wheel | npm / WebAssembly | Android |
| --------- | :-----------------: | :----------: | :---------------: | :-----: |
| `native`  |         Yes         |     Yes      |        Yes        |   Yes   |
| `alembic` |         Yes         |     Yes      |        No         |   Yes   |
| `assimp`  |         Yes         |     Yes      |        Yes        |   Yes   |
| `draco`   |         Yes         |     Yes      |        Yes        |   Yes   |
| `hdf`     |         Yes         |     Yes      |        No         |   No    |
| `occt`    |         Yes         |     Yes      |        Yes        |   Yes   |
| `pdal`    |         Yes         |     Yes      |        No         |   No    |
| `usd`     |         Yes         |      No      |        No         |   No    |
| `vdb`     |         Yes         |      No      |        No         |   No    |
| `webifc`  |         Yes         |     Yes      |        Yes        |   Yes   |

F3D searches for plugins in this order:

1. Search the static plugins.
2. Use the `load-plugins` option when it is a full path.
3. Search the paths specified in the `--plugins-path` CLI option.
4. Search a directory relative to the F3D application: `../lib`.
5. Rely on OS-specific paths (e.g. `LD_LIBRARY_PATH` on Linux or `DYLD_LIBRARY_PATH` on macOS).

You can also try plugins maintained by the community. If you have created a plugin and would like it to be listed here, please submit a pull request.

- **Abaqus**: ODB support by @YangShen398 ([repository](https://github.com/YangShen398/F3D-ODB-Reader-Plugin))

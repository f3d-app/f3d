## Plugins

F3D officially supports plugins for certain file formats. If you installed F3D using the binary
release, there's no need to manually load these plugins when opening your file.
F3D will load them automatically.

However, if you installed F3D using a package manager,
it's possible that the packager chose to bundle the plugins into different packages or
to list plugin dependencies as optional in order to reduce the number of dependencies of the main package.

In order to open a file that requires a plugin, make sure you've installed all necessary
dependencies. You can then specify the `--load-plugins=<path or name>` [option](OPTIONS.md)
in the command line to load your plugin.

Alternatively, you can add your plugin directly in the
[configuration file](CONFIGURATION_FILE.md) if it isn't there already. You can specify one or
multiple plugins in a single comma-separated list, like in the example below:

```
[
{
  "match": ".*(file_extension)",
  "options":
  {
    "load-plugins": "plugin1", "plugin2"
  }
}
]
```

### Supported plugins

F3D supports loading plugins and contains the following plugins:
`native`, `alembic`, `assimp`, `draco`, `hdf`, `occt`, `usd`, `vdb`.

see [the format list](SUPPORTED_FORMATS.md) for more details.

> Note: Native plugin is always loaded automatically.
> Note: If you downloaded the binaries from the release page, it's not necessary to specify manually the plugins above. F3D loads them automatically.

Here is how the plugins are searched (in preceding order):

1. Search the static plugins.
2. Consider the `load-plugins` option given it is a full path.
3. Search in the paths specified in `F3D_PLUGINS_PATH` environment variable.
4. Search in a directory relative to the F3D application: `../lib`.
5. Rely on OS specific paths (e.g. `LD_LIBRARY_PATH` on Linux or `DYLD_LIBRARY_PATH` on macOS).

You can also try plugins maintained by the community. If you have created a plugin and would like it to be listed here, please submit a pull request.

- **Abaqus**: ODB support by @YangShen398 ([repository](https://github.com/YangShen398/F3D-ODB-Reader-Plugin))

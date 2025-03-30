# Desktop Integration

F3D can be integrated in the desktop experience.

## Linux

For Linux desktop integration, F3D rely on mime types files as defined by the [XDG standard](https://specifications.freedesktop.org/mime-apps-spec/mime-apps-spec-latest.html), .thumbnailer file as specified [here](https://wiki.archlinux.org/title/File_manager_functionality#Thumbnail_previews) and .desktop file as specified [here](https://wiki.archlinux.org/title/desktop_entries). Many file managers use this mechanism, including nautilus, thunar, pcmanfm and caja.

The simplest way to obtain desktop integration on Linux is to use a package for your distribution, or the .deb binary release package we provide if compatible with your distribution.
In other cases, the binary release archive can be used like this:

0. Make sure `~/.local/bin` is part of your `PATH`
1. Extract F3D binary release archive in `~/.local/`
2. Update your [mime database](https://linux.die.net/man/1/update-mime-database) pointing to `~/.local/share/mime`
3. Update your [desktop database](https://linuxcommandlibrary.com/man/update-desktop-database) pointing to `~/.local/share/application`

```bash
export PATH=$PATH:~/.local/bin
tar -xzvf f3d-1.3.0-Linux.tar.gz -C ~/.local/
sudo update-mime-database ~/.local/share/mime/
sudo update-desktop-database ~/.local/share/applications
```

F3D supports rendering in environments with limited graphical contexts, including sandboxes or without Xorg running.
In most cases, the default behavior will automatically detect which capabilities are available and use the right rendering backend,
it is possible to specify it using the `--rendering-backend` CLI option.

- If `libEGL.so` is available, you can use: `--rendering-backend=egl`.
- If `libOSMesa.so` is available, you can use: `--rendering-backend=osmesa`.
- When using `--rendering-backend=auto`, F3D will try to load the following libraries in order and use the first one it was able to load and use:
  - `libGLX.so`
  - `libEGL.so`
  - `libOSMesa.so`

You can use that feature for thumbnail generation whenever needed (e.g., Nautilus or other sandboxing file browser),
by modifying the [thumbnail config file](CONFIGURATION_FILE.md), eg:`~/.config/f3d/thumbnail.json`:

```javascript
{
    ".*": {
        "rendering-backend": "egl"
    }
}
```

If you use the portable archive, make sure to extract it somewhere the sandboxing system has access to, eg: Nautilus uses `bwrap` and gives it access to `/usr` but not to `/opt`.

## Windows

For Windows desktop integration, F3D rely on a registered shell extension.

Using the F3D NSIS installer (.exe) is the simplest way to enable thumbnails and integrate F3D on windows.

It is also possible to do it manually when using the zipped binary release archive, on installation, just run:

```
cd C:\path\to\f3d\bin\
regsvr32 F3DShellExtension.dll
```

To remove the shell extension, run:

```
cd C:\path\to\f3d\bin\
regsvr32 /u F3DShellExtension.dll
```

## MacOS

There is no support for thumbnails on MacOS, the .dmg binary release provides automatic file openings.

## Limitations

- TIFF files are excluded from the thumbnailer to avoid conflicts with other better 2D thumbnailers.

# Desktop integration

F3D integrates with the desktop experience.

## Linux

For Linux desktop integration, F3D relies on mime types files as defined by the [XDG standard](https://specifications.freedesktop.org/mime-apps-spec/latest-single/), .thumbnailer file as specified [here](https://wiki.archlinux.org/title/File_manager_functionality#Thumbnail_previews) and .desktop file as specified [here](https://wiki.archlinux.org/title/desktop_entries). Many file managers use this mechanism, including nautilus, thunar, pcmanfm and caja.

The simplest way to obtain desktop integration on Linux is to use a package for your distribution, or the .deb binary release package we provide if compatible with your distribution.
In other cases, use the binary release archive like this:

1. Make sure `~/.local/bin` is part of your `PATH`
2. Extract F3D binary release archive in `~/.local/`
3. Update your [mime database](https://linux.die.net/man/1/update-mime-database) pointing to `~/.local/share/mime`
4. Update your [desktop database](https://linuxcommandlibrary.com/man/update-desktop-database) pointing to `~/.local/share/application`

```bash
export PATH=$PATH:~/.local/bin
tar -xzvf f3d-1.3.0-Linux.tar.gz -C ~/.local/
update-mime-database ~/.local/share/mime/
update-desktop-database ~/.local/share/applications
```

If you use the portable archive, extract it somewhere the sandboxing system has access to, eg: Nautilus uses `bwrap` and gives it access to `/usr` but not to `/opt`.

## Windows

For Windows desktop integration, F3D relies on a registered shell extension.

Using the F3D NSIS installer (.exe) is the simplest way to enable thumbnails and integrate F3D on Windows.

Alternatively, when using the zipped binary release archive, register it manually on installation:

```
cd C:\path\to\f3d\bin\
regsvr32 F3DShellExtension.dll
```

To remove the shell extension, run:

```
cd C:\path\to\f3d\bin\
regsvr32 /u F3DShellExtension.dll
```

## macOS

F3D does not support thumbnails on macOS. The .dmg binary release provides automatic file openings.

## Rendering backends

F3D supports rendering in environments with limited graphical contexts, including sandboxes or without Xorg running on Linux.
By default, F3D detects available capabilities and uses the right rendering backend.
Specify it using the `--rendering-backend` CLI option.

- If EGL is available, use: `--rendering-backend=egl`. F3D attempts to load the following libraries dynamically (in that order):
  - `libEGL.so.1`, `libEGL.so` on Linux
  - `libEGL.dll`, `EGL.dll` on Windows
- If OSMesa is available, use: `--rendering-backend=osmesa`. F3D attempts to load the following libraries dynamically (in that order):
  - `libOSMesa.so.8`, `libOSMesa.so.6`, `libOSMesa.so` on Linux
  - `osmesa.dll` on Windows
  - `libOSMesa.8.dylib`, `libOSMesa.6.dylib`, `libOSMesa.dylib` on macOS
- When using `--rendering-backend=auto`, F3D uses the following logic:
  - On Linux, check if a X server is running. In that case, F3D uses GLX. Otherwise, it tries to load EGL library and fallback on OSMesa if not available.
  - On Windows, check if the OpenGL driver supports OpenGL 3.2. In that case, F3D uses WGL. Otherwise, it tries to load EGL library and fallback on OSMesa if not available.
  - On macOS, always use COCOA. To use OSMesa, you must use `--rendering-backend=osmesa`.

Use that feature for thumbnail generation whenever needed (e.g., Nautilus or other sandboxing file browser),
by creating/modifying your [thumbnail config file](06-CONFIGURATION_FILE.md), eg:`~/.config/f3d/thumbnail.json`:

```json
[
  {
    "options": {
      "rendering-backend": "egl"
    }
  }
]
```

## Limitations

- TIFF files are excluded from the thumbnailer to avoid conflicts with other better 2D thumbnailers.

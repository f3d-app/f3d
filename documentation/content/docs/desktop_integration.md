title: Desktop Integration
---

F3D can be integrated in the desktop experience.

## Linux

For Linux desktop integration, F3D rely on mime types files as defined by the [XDG standard](https://specifications.freedesktop.org/mime-apps-spec/mime-apps-spec-latest.html), .thumbnailer file as specified [here](https://wiki.archlinux.org/title/File_manager_functionality#Thumbnail_previews) and .desktop file as specified [here](https://wiki.archlinux.org/title/desktop_entries). Many file managers use this mechanism, including nautilus, thunar, pcmanfm and caja.

The simplest way to obtain desktop integration on linux is to use a package for your distribution, or the .deb binary package we provide if compatible with your distribution.
In other cases, the binary archive can be used like this:

0. Make sure ~/.local/bin is part of your PATH
1. Extract F3D archive in a TEMP folder
2. move $TEMP/config.json to ~/.config/f3d/
3. copy $TEMP/* to ~/.local/
4. Update your [mime database](https://linux.die.net/man/1/update-mime-database) pointing to ~/.local/share/mime
5. Update your [desktop database](https://linuxcommandlibrary.com/man/update-desktop-database) pointing to ~/.local/share/application

```bash
tar -xzvf f3d-1.2.0-Linux.tar.gz
cd f3d-1.2.0-Linux
mkdir -p ~/.config/f3d/
mv config.json /.config/f3d/
cp -r ./* ~/.local/
sudo update-mime-database ~/.local/share/mime/
sudo update-desktop-database ~/.local/share/applications
```

If you have any issues, read the troubleshooting section.

## Windows

For Windows desktop integration, F3D rely on a registered shell extension.

Using the F3D NSIS installer (.exe) is the simplest way to enable thumbnails and integrate F3D on windows.

It is also possible to do it manually when using the zipped binary release, on installation, just run:

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

There is no support for thumbnails on MacOS, the .dmg binary release should provide automatic file openings.

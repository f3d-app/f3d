title: Desktop Integration
---

F3D can be integrated in the desktop experience in certain cases

## Linux

During installation, F3D will install mime types files as defined by the [XDG standard](https://specifications.freedesktop.org/mime-apps-spec/mime-apps-spec-latest.html), a .thumbnailer file as specified [here](https://wiki.archlinux.org/title/File_manager_functionality#Thumbnail_previews) and a .desktop as specified [here](https://wiki.archlinux.org/title/desktop_entries). Many file managers use this mechanism, including nautilus, thunar, pcmanfm and caja. Make sure to update the mime types database using [update-mime-database](https://linux.die.net/man/1/update-mime-database) as well as the desktop entries database using [update-desktop-database](https://linuxcommandlibrary.com/man/update-desktop-database).

## Windows

Using the F3D NSIS installer is the simplest way to enable thumbnails on windows, you can find it in the release section. It will automatically register it when installing F3D and unregister it when uninstalling F3D.
It is also possible to manually register it using `regsvr32 F3DShellExtension.dll`. To unregister it, use `regsvr32 /u F3DShellExtension.dll`.
The NSIS installer also register/unregister extensions support on installation/deinstallation.

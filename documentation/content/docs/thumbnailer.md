title: Thumbnailer 
---

F3D can be used as a thumbnailer in certain cases

## Linux

During instalation, F3D will install mime types files as defined by the [XDG standard](https://specifications.freedesktop.org/mime-apps-spec/mime-apps-spec-latest.html) and a thumbnailer file as specified [here](https://wiki.archlinux.org/title/File_manager_functionality#Thumbnail_previews). Many file managers use this mechanism, including nautilus, thunar, pcmanfm and caja. Make sure to update the mime types database using [update-mime-database](https://linux.die.net/man/1/update-mime-database).

## Windows

Using the F3D NSIS installer is the simplest way to enable thumbnails on windows, you can find it in the release section. It will automatically register it when installing F3D and unregister it when uninstalling F3D.
It is also possible to manually register it using `regsvr32 F3DShellExtension.dll`. To unregister it, use `regsvr32 /u F3DShellExtension.dll`.

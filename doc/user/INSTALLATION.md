# Installation

## Current release packages (3.2.0)

| Platform               | Files                                                                                                                       |
| ---------------------- | --------------------------------------------------------------------------------------------------------------------------- |
| Windows (installer)    | [F3D-3.2.0-Windows.exe](https://github.com/f3d-app/f3d/releases/download/v3.2.0/F3D-3.2.0-Windows-x86_64-raytracing.exe)    |
| Windows (portable)     | [F3D-3.2.0-Windows.zip](https://github.com/f3d-app/f3d/releases/download/v3.2.0/F3D-3.2.0-Windows-x86_64-raytracing.zip)    |
| MacOS (Silicon)        | [F3D-3.2.0-macOS-arm64.dmg](https://github.com/f3d-app/f3d/releases/download/v3.2.0/F3D-3.2.0-macOS-arm64-raytracing.dmg)   |
| MacOS (Intel)          | [F3D-3.2.0-macOS-x86_64.dmg](https://github.com/f3d-app/f3d/releases/download/v3.2.0/F3D-3.2.0-macOS-x86_64-raytracing.dmg) |
| Linux (Debian package) | [F3D-3.2.0-Linux.deb](https://github.com/f3d-app/f3d/releases/download/v3.2.0/F3D-3.2.0-Linux-x86_64-raytracing.deb)        |
| Linux (portable)       | [F3D-3.2.0-Linux.tar.xz](https://github.com/f3d-app/f3d/releases/download/v3.2.0/F3D-3.2.0-Linux-x86_64-raytracing.tar.xz)  |

### Notes

- MacOS package is not signed, see the [troubleshooting](LIMITATIONS_AND_TROUBLESHOOTING.md#macos) section for a workaround if needed.
- MacOS packages require macOS >= 12.0
- Linux packages require GLIBC >= 2.31

## Alternatives

You can find other (older, smaller, checksum, RCs) release related packages and files on the [releases page](https://github.com/f3d-app/f3d/releases).
You can also find there the [nightly release](https://github.com/f3d-app/f3d/releases/tag/nightly) to test the development version of F3D.
See the [desktop integration](DESKTOP_INTEGRATION.md) section in order actually integrate the binary release in your desktop.
Alternatively, you can build F3D yourself by following the [build](../dev/BUILD.md) guide.

You can also find packages for the following repositories:

### Windows

- [Scoop](https://scoop.sh/#/apps?q=f3d&s=0&d=1&o=true)
- [winget](https://winstall.app/apps/f3d-app.f3d)

### MacOS

- [Homebrew](https://formulae.brew.sh/formula/f3d)

### Linux

- [Ubuntu](https://packages.ubuntu.com/search?keywords=f3d&searchon=names&exact=1&suite=all&section=all)
- [Debian](https://packages.debian.org/search?keywords=f3d&searchon=names&exact=1&suite=all&section=all)
- [Arch](https://archlinux.org/packages/extra/x86_64/f3d/)
- [Fedora](https://packages.fedoraproject.org/pkgs/f3d/f3d/)
- [openSUSE](https://software.opensuse.org/package/f3d)
- [OpenMandriva](https://github.com/OpenMandrivaAssociation/f3d)
- [Mageia](https://madb.mageialinux-online.org/rpmshow?rpm=f3d&repo=9-SRPMS-core-release&distribution=9&architecture=x86_64&graphical=0)
- [ALT Linux](https://packages.altlinux.org/en/sisyphus/srpms/f3d/)
- [Guix](https://hpc.guix.info/package/f3d)
- [Flathub](https://flathub.org/apps/details/io.github.f3d_app.f3d)
- [NixOS](https://search.nixos.org/packages?query=f3d)

### Other

- [FreeBSD](https://cgit.freebsd.org/ports/tree/graphics/f3d)
- [Spack](https://packages.spack.io/package.html?name=f3d)
- [pypi](https://pypi.org/project/f3d/) (library only)

### libf3d based tools

- [Exhibit](https://flathub.org/apps/io.github.nokse22.Exhibit)
- [f3dviewer](https://github.com/ccseer/f3dviewer)

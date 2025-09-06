[![CI](https://img.shields.io/github/actions/workflow/status/f3d-app/f3d/ci.yml?label=CI&logo=github)](https://github.com/f3d-app/f3d/actions/workflows/ci.yml) [![Packaging](https://img.shields.io/github/actions/workflow/status/f3d-app/f3d-superbuild/nightly.yml?label=Packaging&logo=github)](https://github.com/f3d-app/f3d-superbuild) [![codecov](https://codecov.io/gh/f3d-app/f3d/branch/master/graph/badge.svg?token=siwG82IXK7)](https://codecov.io/gh/f3d-app/f3d) [![Downloads](https://img.shields.io/github/downloads/f3d-app/f3d/total.svg)](https://github.com/f3d-app/f3d/releases) [![Sponsors](https://img.shields.io/static/v1?label=Sponsor&message=%E2%9D%A4&logo=GitHub&color=%23fe8e86)](https://github.com/sponsors/f3d-app) [![Discord](https://discordapp.com/api/guilds/1046005690809978911/widget.png?style=shield)](https://discord.f3d.app) [![Contributor Covenant](https://img.shields.io/badge/Contributor%20Covenant-2.1-4baaaa.svg)](CODE_OF_CONDUCT.md)

# F3D - Fast and minimalist 3D viewer

By Michael Migliore and Mathieu Westphal.

<img src="https://raw.githubusercontent.com/f3d-app/f3d/master/resources/logo.svg" align="left" width="20px"/>
F3D (pronounced `/fɛd/`) is a fast and minimalist 3D viewer desktop application. It supports many file formats, from digital content to scientific datasets (including glTF, USD, STL, STEP, PLY, OBJ, FBX, Alembic), can show animations and support thumbnails and many rendering and texturing options including real time physically based rendering and raytracing.
<br clear="left"/>

It is fully controllable from the command line and support configuration files. It can provide thumbnails, support interactive hotkeys, drag&drop and integration into file managers.

F3D also contains the libf3d, a simple library to render meshes, with a C++17 API, Python Bindings, and experimental Java and Javascript bindings.

<img src="https://media.githubusercontent.com/media/f3d-app/f3d-media/95b76c22d927bb24759bbe0246b6260121f2933b/media/typical.png" width="640" />

_A typical render by F3D_

<img src="https://user-images.githubusercontent.com/3129530/194735261-dd6f1c1c-fa57-47b0-9d27-f735d18ccd5e.gif" width="640" />

_Animation of a glTF file within F3D_

<img src="https://user-images.githubusercontent.com/3129530/194735272-5bcd3e7c-a333-41f5-8066-9b0bec9885e8.png" width="640" />

_A direct scalars render by F3D_

See the [gallery](doc/GALLERY.md) for more images, take a look at the [changelog](doc/CHANGELOG.md) or go to the [install guide](doc/user/INSTALLATION.md) to download and install F3D!

You can even use F3D directly in your [browser](https://f3d.app/web)!

If you need any help or want to discuss with other F3D users and developers, head over to our [discord](https://discord.f3d.app).

# Quickstart

Open a file directly in F3D or from the command line by running:

```
f3d /path/to/file.ext
```

Optionally, append `--output=/path/to/img.png` to save the rendering into an image file.

See the [Quickstart Guide](doc/user/QUICKSTART.md) for more information about getting started with F3D.

# Documentation

- To get started, please take a look at the [user documentation](doc/user/README_USER.md).
- If you need any help, are looking for a feature or found a bug, please open an [issue](https://github.com/f3d-app/f3d/issues).
- If you want to use the libf3d, please take a look at its [documentation](doc/libf3d/README_LIBF3D.md).
- If you want to build F3D, please take a look at the [developer documentation](doc/dev/README_DEV.md).

# Support

F3D needs your help!

If you can, please consider sponsoring F3D. Even a small donation would help us offset the recurring maintenance costs.
With enough sponsors we would be able to make F3D grow faster and stronger! Read more about it [here](doc/user/SPONSORING.md).

If you are an industry user of F3D and want to make sure it can keep growing and being maintained, [please reach out](doc/user/SPONSORING.md#industry-sponsors)!

In any case, please star it on github and share the word about it!

# Vision

As a minimalist 3D viewer F3D aims to:

- Support as many 3D file formats as possible
- Support many types of renderings (textures, edges, etc... ) and visualizations (meshes, volumic, point sprites)
- Support any and all use-cases dealing with 3D datasets
- Let any user easily and quickly view any model with good defaults
- Be as configurable as possible
- Be fully controllable from the command line and configuration file
- Be usable non-interactively
- Be as modular as possible to be built with a small number of dependencies

but there is no plan to:

- Provide a classic mouse-based UI, with menus and buttons
- Provide data processing tools
- Provide export feature

# Contributing

F3D as a community-driven, inclusive and beginner-friendly project. We love to see how the project is growing thanks to the contributions from the community. We would love to see your face in the list below! If you want to contribute to F3D, you are very welcome to! Take a look at our [contribution documentation](CONTRIBUTING.md), [governance documentation](doc/dev/GOVERNANCE.md) and [code of conduct](CODE_OF_CONDUCT.md).

<a href="https://github.com/f3d-app/f3d/graphs/contributors">
  <img src="https://contrib.rocks/image?repo=f3d-app/f3d" />
</a>

# Acknowledgments

F3D was initially created by [Kitware SAS](https://www.kitware.eu/) and is relying on many awesome open source projects, including [VTK](https://vtk.org/), [OCCT](https://dev.opencascade.org/), [Assimp](https://www.assimp.org/), [Alembic](http://www.alembic.io/), [Draco](https://google.github.io/draco/), [OpenUSD](https://openusd.org/release/index.html), [OpenVDB](https://www.openvdb.org/), [OSPRay](https://www.ospray.org/) and [ImGui](https://github.com/ocornut/imgui/).

# License

F3D can be used and distributed under the 3-Clause BSD License, see the [license](LICENSE.md).
F3D integrate the sources of other libraries and tools, all under permissive licenses, see the [third party licenses](doc/THIRD_PARTY_LICENSES.md).
F3D packages relies on other libraries and tools, all under permissive licenses, all listed in the respective packages.

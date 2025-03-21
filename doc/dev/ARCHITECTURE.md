# F3D Architecture

F3D is structured in different parts, interacting with each others and with F3D dependencies.
This architecture is reflected by the directories organisation.

- **application**: the code of the F3D application itself, see below
- cmake: cmake macros and functions, used by the CMake build system
- doc: this very documentation
- examples: examples usage of the libf3d and plugin framework in python and C++
- external: dependencies that are included directly in the code
- java: [java bindings](../libf3d/LANGUAGE_BINDINGS.md#java-experimental) and associated tests.
- **library**: the [libf3d](../libf3d/README_LIBF3D.md) itself, see below
- **plugins**: all the [plugins](../libf3d/PLUGINS.md) providing different readers, see below
- python: [python bindings](../libf3d/LANGUAGE_BINDINGS.md#python) and tests
- resources: all non code, non doc, like icon, configs and such
- testing: all testing related resources, does not contain the test themselves
- **vtkext**: extensions to VTK and related tests, see below
- webassembly: [webassembly/javascript bindings](../libf3d/LANGUAGE_BINDINGS.md#javascript-experimental) and [F3DWeb](https://f3d.app/web/) application code
- winshellext: shell extension for Windows, provide [thumbnails for Windows](../user/DESKTOP_INTEGRATION.md#windows)

Here is diagram explaining how some of these parts interact together:

```
   ┌────────────────┐              ┌───────────────┐
   │                │              │               │
   │  application   │◄────uses─────│  winshellext  │
   │                │              │               │
   └────────────────┘              └───────────────┘
           │
           │                                   ┌──────────┐
           │                                   │          │
       depends on                     ┌─wraps──│  python  │
           │                          │        │          │
           │                          │        └──────────┘
           │         ┌───────────┐    │        ┌──────────┐
           └────────►│           │◄───┘        │          │
                     │  library  │◄─────wraps──│   java   │
      ┌───loads──────│           │◄───┐        │          │
      │              └───────────┘    │        └──────────┘
      ▼                    │          │        ┌──────────┐
┌───────────┐              │          │        │          │
│           │          depends on     └─wraps──│   wasm   │
│  plugins  │              │                   │          │
│           │              ▼                   └──────────┘
└───────────┘     ┌──────────────────┐
      │           │      vtkext      │
      │           ├────────┬─────────┤
     depends─on──►│ public │ private │
                  └────────┴─────────┘
```

## vtkext

`vtkext` contains two [VTK modules](https://docs.vtk.org/en/latest/api/cmake/ModuleSystem.html) that are used extensively in the libf3d. The public one is used in the plugins.

`public` is a VTK module that contains classes and utilities that can be installed as part of the `plugin_sdk` and used by plugins, including externals plugins. `vtkF3DImporter` is a class
that is specifically made for plugin developers to inherit their importers from. The documentation of this module can be found [here](https://f3d.app/doc/libf3d/vtkext_doxygen/).

`private` is a VTK module that contains many classes and utilities used by the libf3d to provide all features of F3D, especially the rendering, interactions and UI.
A notable class is `vtkF3DRenderer` that is responsible to actually add the different actors in the 3D scene.

Each of these modules also contains [tests](TESTING.md#vtkextensions-layer) in the `Testing` directory.

## plugins

`plugins` contains [libf3d plugins](../libf3d/PLUGINS.md) that are provided by default in the F3D packages. Each of these plugins correspond to a specific dependency and are named accordingly. Each of these plugins provide access to specific readers for specific formats. Without plugins, F3D and the libf3d would not be able to open any file. These plugins can be loaded statically or dynamically, which makes the dependencies truly optional if needed.

## library

`library` contains the code of the libf3d. It is a C++ library with a very limited API surface and larger, private, implementation.
Most classes in the libf3d are split in two. A public part that contains mostly the public API, and a private part, suffixed "\_impl", that implements that public API
and also contains hidden methods used to communicate between classes, especially in regards to VTK symbols.

Logically, it is structured in 3 parts, `public` which contains the public API header files and are all installed, `private` which contains the implementation classes headers files and `src` that contains the source files of all the classes, public and private.

There is also a dedicated `testing` directory which contains the [unit and functional testing](TESTING.md#library-layer) of the libf3d.

It also contains the `options.json` file, which is the file used to generate all [options](../libf3d/OPTIONS.md) code.

## application

`application` contains the code of the F3D application itself. It relies of course on the libf3d to implement all the applicative logic.
The most important class in the `F3DStarter` which contains most of the top logic on the application. `F3DOptionsTools` is also notable as it handles most of
the command line options logic.

There is also a dedicated `testing` directory which contains all of the [applicative testing](TESTING.md#application-layer) of the F3D application as well as many functional testing of the libf3d.

## Other f3d-app repositories

Although almost everything is contained in the [f3d-app/f3d](https://github.com/f3d-app/f3d) repository, other repositories in the [f3d-app](https://github.com/f3d-app) organisation are handling certains specific tasks in the F3D ecosystem.

- The [f3d-superbuild](https://github.com/f3d-app/f3d-superbuild) handles the packaging and the creation of the binaries provided in the [releases page](https://github.com/f3d-app/f3d/releases).
- [f3d-media](https://github.com/f3d-app/f3d-media) backups all images and video used in this documentation
- A collection of actions: [sccache-setup](https://github.com/f3d-app/sccache-setup-action), [lfs-data-cache](https://github.com/f3d-app/lfs-data-cache-action) and [install-mesa-windows](https://github.com/f3d-app/install-mesa-windows-action) used by the CI of F3D
- A collection of [docker files](https://github.com/f3d-app/f3d-docker-images) used for generating docker images used by the CI of F3D

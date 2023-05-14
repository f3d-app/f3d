# Limitations

Here is a non exhaustive list of F3D limitations:

* No support for specifying manual lighting in the default scene.
* Multiblock (.vtm, .gml) support is partial, non-surfacic data will be converted into surfaces.
* Animation support with full scene data format require VTK >= 9.0.20201016.
* Full drag and drop support require VTK >= 9.0.20210620
* Drag and drop interaction cannot be recorded nor played back.
* Volume rendering and HDRI support requires a decent GPU.

## Assimp
FBX, DAE, OFF, and DXF file formats rely on [Assimp](https://github.com/assimp/assimp) library. It comes with some known limitations:
- PBR materials are not supported for FBX file format.
- Complex animations are not working very well with Assimp 5.1, it's recommended to use Assimp 5.0 for this use case.
- Some files can be empty, crash, or show artifacts.
- DXF support is very limited: only files with polylines and 3D faces are displayed.

## Alembic
ABC file formats rely on [Alembic](https://github.com/alembic/alembic) library. It comes with some known limitations:
- Supports only simple polygonal geometry.
- Does not support ArbGeomParam feature in Alembic.
- Does not support Subdivision Meshes.
- Does not support Materials.

# Troubleshooting

## General
> I have built F3D with raytracing support but the denoiser is not working.

Make sure that VTK has been built with *OpenImageDenoise* support (`VTKOSPRAY_ENABLE_DENOISER` option).

> My model shows up all wrong, with inverted surfaces everywhere.

Your data probably contains some translucent data for some reason, turn on translucency support by pressing `P` or using the `-p` command line option.

## Linux

> I have a link error related to `stdc++fs` not found.

With some C++ STD library version, explicit linking to `stdc++fs` is not supported. We provide a CMake option `F3D_LINUX_APPLICATION_LINK_FILESYSTEM` that you can set to `OFF` to workaround this issue.

> Thumbnails are not working in my file manager.

 * Check that your file manager supports the thumbnailer mechanism.
 * Check that you have updated your mime type database.
 * If all fails, remove your `.cache` user dir and check that `pcmanfm` thumbnails are working.
  * If they are working, then it is an issue specific to your file manager (see below for a potential work around).
  * If only a few format have working thumbnails, then it is an issue with the mime types database.
  * If no formats have working thumbnails, then it is an issue with the `f3d-plugin-xxx.thumbnailer` files.
  * If only big file do not have thumbnails, this is intended, you can modify this behavior in the `thumbnail.d/10_global.json` configuration folder using the `max-size` option.

Some file managers (eg: Nautilus) are using sandboxing for thumbnails, which the F3D standard binary release does not support as it needs
access to the Xorg server to render anything.
In that case, you want to use the headless version of F3D that rely on EGL instead of Xorg, available in the [releases page](https://github.com/f3d-app/f3d/releases).
If you use the portable archive, make sure to extract it somewhere the sandboxing system has access to, eg: Nautilus uses `bwrap` and gives it access to `/usr` but not to `/opt`.

## Windows

> Using the portable binary release, I'm unable to run F3D because Windows warns about a security risk

F3D is not signed by Microsoft yet so Windows shows this warning. Just click on the "more" button to be able to run F3D.

> After installing F3D or registering the shell extension, my explorer is broken.

Unregister the shell extension by running:

```
cd C:\path\to\f3d\bin\
regsvr32 /u F3DShellExtension.dll
```

> I use F3D in a VM, the application fails to launch.

OpenGL applications like F3D can have issues when launched from Windows Server or from a guest Windows because the access to the GPU is restricted.
You can try to use a software implementation of OpenGL, called [Mesa](https://github.com/pal1000/mesa-dist-win/releases).
 * Download the latest `release-msvc`.
 * copy `x64/opengl32.dll`, `libgallium_wgl.dll` and `x64/libglapi.dll` in the same folder as `f3d.exe`.
 * set the environment variable `MESA_GL_VERSION_OVERRIDE` to 4.5.
 * run `f3d.exe`.

> I run f3d from the command prompt and my Unicode characters are not displayed properly.

Set the codepage to UTF-8, run `chcp 65001`.

## macOS

> Using the binary release, I'm unable to run F3D because macOS warns about F3D not being signed.

F3D is not signed by Apple yet so macOS shows this warning. To run F3D, right click on the app and click "open", then click "open" again to be able to run F3D.

> I'm unable to get coloring right with step files

F3D on macOS does not support coloring on cells because of a [VTK issue](https://gitlab.kitware.com/vtk/vtk/-/issues/18969)

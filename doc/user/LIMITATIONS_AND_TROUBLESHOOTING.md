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

## Linux

> I have a link error related to `stdc++fs` not found.

With some C++ STD library version, explicit linking to `stdc++fs` is not supported. We provide a CMake option `F3D_LINUX_APPLICATION_LINK_FILESYSTEM` that you can set to `OFF` to workaround this issue.

### Thumbnails
> Thumbnails are not working in my file manager.

 * Check that your file manager supports the thumbnailer mechanism.
 * Check that you have updated your mime type database.
 * If all fails, remove your `.cache` user dir and check that `pcmanfm` thumbnails are working.
  * If they are working, then it is an issue specific to your file manager (see below for a potential work around).
  * If only a few format have working thumbnails, then it is an issue with the mime types database.
  * If no formats have working thumbnails, then it is an issue with the `f3d.thumbnailer` file.
  * If only big file do not have thumbnails, this is intended, you can modify this behavior in the `thumbnail.json` configuration file using the `max-size` option.

### Sandboxing
Some file managers (eg: Nautilus) are using sandboxing for thumbnails, which the F3D binary release does not support as it needs
access to the Xorg server for rendering anything.
A work around to this issue is to use a virtual Xorg server like Xephyr or Xvfb in the `f3d.thumbnailer` file.
Here is how your `Exec` line should look to use `xvfb-run`. Keep in mind running xvfb can be very slow.

`Exec=xvfb-run f3d --dry-run -sta --no-background --output=%o --resolution=%s,%s %i`

Another workaround is to build VTK with EGL or osmesa support and then build F3D yourself against
this custom VTK build.

## Windows
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

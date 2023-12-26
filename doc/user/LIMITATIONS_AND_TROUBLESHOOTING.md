# Limitations

Here is a non exhaustive list of F3D limitations:

* No support for specifying manual lighting in the default scene apart from using `--light-intensity` option.
* Multiblock (.vtm, .gml) support is partial, non-surfacic data will be converted into surfaces.
* Animation support with full scene data format require VTK >= 9.0.20201016.
* Full drag and drop support require VTK >= 9.0.20210620
* Drag and drop interaction cannot be recorded nor played back.
* Volume rendering and HDRI support requires a decent GPU.
* The `--camera-zoom-factor` option require VTK >= 9.3.0

## Assimp
FBX, DAE, OFF, and DXF file formats rely on [Assimp](https://github.com/assimp/assimp) library. It comes with some known limitations:
- PBR materials are not supported for FBX file format.
- Complex animations are not working very well with Assimp 5.1, it's recommended to use Assimp 5.0 for this use case.
- Only one animation can be shown at a time, showing all animations is not supported yet.
- Some files can be empty, crash, or show artifacts.
- DXF support is very limited: only files with polylines and 3D faces are displayed.

## Alembic
ABC file formats rely on [Alembic](https://github.com/alembic/alembic) library. It comes with some known limitations:
- Supports only simple polygonal geometry.
- Does not support ArbGeomParam feature in Alembic.
- Does not support Subdivision Meshes.
- Does not support Materials.
- Does not support Animations.

## USD
USD file formats rely on [OpenUSD](https://github.com/PixarAnimationStudios/OpenUSD) library. It comes with some known limitations:
- Skinning is slow and baked on the CPU.
- Does not support Face-varying attributes.

## VDB
VDB file formats rely on [OpenVDB](https://github.com/AcademySoftwareFoundation/openvdb) and VTK libraries. It currently comes with some known limitations:
- Not tested in continuous integration on macOS and not shipped in the binary release for macOS arm.
- VDB Grid files are opened with a hard-coded 0.1 sampling rate.

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

> When I install F3D, I can't add F3D to the path and the installer complains that: `PATH too long installer unable to modify PATH!`.

There are too many entry in your PATH environment variable, remove the ones you do not use.

> When I try to update F3D, the installer fails with: `Error opening file for writing: F3DShellExtension.dll`

Your previous F3D installation is current being used to generate thumbnails, wait for a bit or reboot your computer before updating again.

## macOS

> I'm unable to use the binary release, macOS complains the file is invalid or corrupted

Our macOS binary package are not signed, to work around this, you can simply run the following command:

```
xattr -cr /Applications/F3D.app
```

Then F3D should work as expected.

> Using the silicon binary release, I cannot find the raytracing feature or open exodus files

F3D raytracing and exodus plugin are not working on macOS silicon yet, see this [issue](https://github.com/f3d-app/f3d/issues/976).

> I'm unable to get coloring right with step files

F3D on macOS does not support coloring on cells because of a [VTK issue](https://gitlab.kitware.com/vtk/vtk/-/issues/18969).

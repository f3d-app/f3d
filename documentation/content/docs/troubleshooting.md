title: Troubleshooting
---

## General
> I have built F3D with raytracing support but the denoiser is not working.

Be sure that VTK has been built with *OpenImageDenoise* support (`VTKOSPRAY_ENABLE_DENOISER` option).

## Linux
> Thumbnails are not working in my file manager.

 * Check that your file manager supports the thumbnailer mechanism.
 * Check that you have updated your mime type database.
 * If all fails, remove your .cache user dir and check that pcmanfm thumbnails are working.
  * If they are working, then it is an issue specific to your file manager.
  * If only a few format have working thumbnails, then it is an issue with mime types
  * If no formats have working thumbnails, then it is an issue with the f3d.thumbnailer file

## Windows
> I use F3D in a VM, the application fails to launch.

OpenGL applications like F3D can have issues when launched from a guest Windows because the access to the GPU is restricted.
You can try to use a software implementation of OpenGL, called [Mesa](https://github.com/pal1000/mesa-dist-win/releases).
 * Download the latest `release-msvc`.
 * copy `x64/OpenGL32.dll` and `x64/libglapi.dll` in the same folder as `f3d.exe`.
 * set the environment variable `MESA_GL_VERSION_OVERRIDE` to 4.5.
 * run `f3d.exe`.

> I run f3d from the command prompt and my Unicode characters are not displayed properly.

Set the codepage to UTF-8, run `chcp 65001`.

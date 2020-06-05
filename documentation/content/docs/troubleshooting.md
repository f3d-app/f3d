title: Troubleshooting
---

## Windows
> I have installed F3D using the provided installer but when launched, I have an error with unfound DLL libraries.

You need to install [Visual C++ Redistributable for Visual Studio 2015](https://www.microsoft.com/en-us/download/details.aspx?id=48145), select the `x64` version.

> I use F3D in a VM, the application fails to launch

OpenGL applications like F3D can have issues when launched from a guest Windows because the access to the GPU is restricted.\
You can try to use a software implementation of OpenGL, called [Mesa](https://github.com/pal1000/mesa-dist-win/releases).\
 * Download the lastest `release-msvc`
 * copy `x64/OpenGL32.dll` and `x64/libglapi.dll` in the same folder as `f3d.exe`.
 * set the environment variable `MESA_GL_VERSION_OVERRIDE` to 4.5
 * run `f3d.exe`

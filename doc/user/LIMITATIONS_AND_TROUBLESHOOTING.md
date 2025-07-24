# Limitations

Here is a non exhaustive list of F3D limitations:

- No support for specifying manual lighting in the default scene apart from using `--light-intensity` option.
- Multiblock (.vtm, .gml) support is partial, non-surfacic data will be converted into surfaces.
- Drag and drop interaction cannot be recorded nor played back.
- Volume rendering and HDRI support requires a decent GPU.
- The `--camera-zoom-factor` option require VTK >= 9.3.0
- Information about the failure to load a file is not provided before VTK >= 9.4.0

## Assimp

FBX, DAE, OFF, DXF, X and 3MF file formats rely on [Assimp](https://github.com/assimp/assimp) library. It comes with some known limitations:

- PBR materials are not supported for FBX file format.
- Some files can be empty, crash, or show artifacts.
- DXF support is very limited: only files with polylines and 3D faces are displayed.
- 3MF files may crash at exit (issue in Assimp: https://github.com/assimp/assimp/issues/5328)
- Only support RBGA 8-bits embedded textures

## Alembic

ABC file formats rely on [Alembic](https://github.com/alembic/alembic) library. It comes with some known limitations:

- Supports only simple polygonal geometry.
- Does not support ArbGeomParam feature in Alembic.
- Does not support Subdivision Meshes.
- Does not support Materials.

## USD

USD file formats rely on [OpenUSD](https://github.com/PixarAnimationStudios/OpenUSD) library. It comes with some known limitations:

- Skinning is slow and baked on the CPU.
- Does not support Face-varying attributes.
- The `usd` plugin is not shipped in the python wheels yet.

## VDB

VDB file formats rely on [OpenVDB](https://github.com/AcademySoftwareFoundation/openvdb) and VTK libraries. It currently comes with some known limitations:

- VDB Grid files are opened with a hard-coded 0.1 sampling rate.
- The `vdb` plugin is not shipped in the python wheels yet.

## Gaussian splatting

Gaussian splatting (option `--point-sprites-type=gaussian`) needs depth sorting which is done internally using a compute shader. This requires support for OpenGL 4.3 which is not supported by macOS and old GPUs/drivers.

# Troubleshooting

## General

> I have built F3D with raytracing support but the denoiser is not working.

Make sure that VTK has been built with _OpenImageDenoise_ support (`VTKOSPRAY_ENABLE_DENOISER` option).

> My model shows up all wrong, with inverted surfaces everywhere.

Your data probably contains some translucent data for some reason, turn on translucency support by pressing `P` or using the `-p` command line option.

> My 3D Gaussian Splatting data in .ply format isn't opened properly.

.ply is a generic file format, we cannot assumes it's a 3DGS, thus we do not give good presets for this specific use case.
It's recommended to use these options: `--point-sprites-size=1 --point-sprites-type=gaussian -soyk`, but you can also add them in your [config file](CONFIGURATION_FILE.md).

## Linux

> I have a link error related to `stdc++fs` not found or I'm unable to run F3D due to filesystem errors.

With some C++ STD library version, explicit linking to `stdc++fs` is not supported or required. We provide a CMake option `F3D_LINUX_LINK_FILESYSTEM` that you can set to the desired value to workaround this issue.

> I have a link error related to undefined reference to symbol of `libatomic`.

The GCC flag `-latomic` is not being added automatically with specific architectures, like `armel` and `RISCV64`. We provide a CMake option `F3D_LINUX_LIBRARY_LINK_ATOMIC` that you can set to `ON` to workaround this issue.

> Thumbnails are not working in my file manager.

- Check that your file manager supports the thumbnailer mechanism.
- Check that you have updated your mime type database.
- If all fails, remove your `.cache` user dir and check that `pcmanfm` thumbnails are working.
- If they are working, then it is an issue specific to your file manager (see below for a potential work around).
- If only a few format have working thumbnails, then it is an issue with the mime types database.
- If no formats have working thumbnails, then it is can be an issue with sandboxing or with the `f3d-plugin-xxx.thumbnailer` files.
- If only big file do not have thumbnails, this is intended, you can modify this behavior in the `thumbnail.d/05_all.json` configuration directory using the `max-size` option.

> `--rendering-backend` CLI option is not working as expected

Dynamically switching rendering backend require VTK 9.4.0, you may need to update VTK or to use our [binary release](INSTALLATION.md).

> I'm unable to link C++ examples against my local F3D install directory, it complains about missing VTK symbols

Make sure to have VTK installed in your system or to add VTK install lib directory to `LD_LIBRARY_PATH`.

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

- Download the latest `release-msvc`.
- copy `x64/opengl32.dll`, `libgallium_wgl.dll` and `x64/libglapi.dll` in the same directory as `f3d.exe`.
- set the environment variable `MESA_GL_VERSION_OVERRIDE` to 4.5.
- run `f3d.exe`.

> I run f3d from the command prompt and my Unicode characters are not displayed properly.

Set the codepage to UTF-8, run `chcp 65001`.

> When I install F3D, I can't add F3D to the path and the installer complains that: `PATH too long installer unable to modify PATH!`.

There are too many entry in your PATH environment variable, remove the ones you do not use.

> When I try to update F3D, the installer fails with: `Error opening file for writing: F3DShellExtension.dll`

Your previous F3D installation is current being used to generate thumbnails, wait for a bit or reboot your computer before updating again.

> When running F3D without interactivity (e.g. `f3d.exe --version`) from CLI, nothing is displayed.

It's a limitation from the Windows "subsystem". Use `f3d-console.exe` instead.

> When playing an animation, the animation appear to move too slowly

This is a limitation of older VTK version before VTK 9.5.0, use our [binaries](INSTALLATION.md) or compile a recent version of VTK.

## macOS

> I'm unable to use the binary release, macOS complains the file is invalid or corrupted

Our macOS binary package are not signed, to work around this, you can simply run the following command:

```
xattr -cr /Applications/F3D.app
```

Then F3D should work as expected.

## Python

> I can't find `usd` and `vdb` plugins after installing f3d with pip

The `usd` and `vdb` plugins are not shipped in the python wheels for now, you can compile F3D yourself with them though.

> Raytracing does not work after installing f3d with pip

F3D raytracing feature is not shipped in the python wheels for now, you can compile F3D yourself with it though.

# Limitations and troubleshooting

## Limitations

This is a non exhaustive list of F3D limitations:

- F3D does not support manual lighting in the default scene, apart from the `--light-intensity` option.
- Drag and drop interaction cannot be recorded or played back.
- Volume rendering and HDRI support require a decent GPU.
- Streaming requires different VTK versions depending upon the format to read.
- The `ui.dpi_aware` option and CLI `--dpi-aware` are only supported on Windows platform.

### Assimp

FBX, DAE, OFF, DXF, X, 3MF and AMF file formats rely on [Assimp](https://github.com/assimp/assimp) library. They have some known limitations:

- PBR materials are not supported for FBX file format.
- Some files can be empty, crash, or show artifacts.
- DXF support is very limited: F3D displays only files with polylines and 3D faces.
- 3MF files may crash at exit (issue in Assimp: https://github.com/assimp/assimp/issues/5328)
- They support only RBGA 8-bits embedded textures.

### Alembic

ABC file formats rely on [Alembic](https://github.com/alembic/alembic) library. They have some known limitations:

- Supports only simple polygonal geometry.
- Does not support ArbGeomParam feature in Alembic.
- Does not support Subdivision Meshes.
- Does not support Materials.

### USD

USD file formats rely on [OpenUSD](https://github.com/PixarAnimationStudios/OpenUSD) library. They have some known limitations:

- Skinning is slow and baked on the CPU.
- Does not support Face-varying attributes.
- The `usd` plugin is not shipped in the Python wheels yet.

### VDB

VDB file formats rely on [OpenVDB](https://github.com/AcademySoftwareFoundation/openvdb) and VTK libraries. They have some known limitations:

- The `vdb` plugin is not shipped in the Python wheels yet.

### Gaussian splatting

Gaussian splatting (option `--point-sprites=gaussian`) needs depth sorting, which F3D does internally using a compute shader. This requires support for OpenGL 4.3, which is not supported by macOS and old GPUs/drivers.

## Troubleshooting

### General

> I have built F3D with raytracing support but the denoiser is not working.

Build VTK with _OpenImageDenoise_ support (`VTKOSPRAY_ENABLE_DENOISER` option).

> My model shows up all wrong, with inverted surfaces everywhere.

Your data probably contains translucent data. Turn on translucency support by pressing `P` or using the `-p` command-line option.

> My 3D Gaussian Splatting data in .ply format isn't opened properly.

.ply is a generic file format, so F3D cannot assume it's a 3DGS file and does not provide presets for this specific use case.
Use these options: `--point-sprites-size=1 --point-sprites=gaussian -syk`. You can also add them in your [config file](06-CONFIGURATION_FILE.md).

> I can't use hotkeys/bindings with F row keys

Some keyboards have layered multimedia keys, so F keys (F12 for example) don't use the normal function code by default. To use the F code, press <kbd>Fn</kbd> in combination with the F key. For example, to take a minimal screenshot, the hotkey combination on macOS is <kbd>Cmd</kbd>+<kbd>Fn</kbd>+<kbd>F12</kbd>.

> My step file doesn't respect the `--color` option

When rendering step files, F3D does not respect the `--color` option because the configuration file uses scalar coloring by default. To override this behavior, disable scalar coloring when specifying the color option. For example: `--color=Blue --scalar-coloring=no`.

Alternatively, add the following in your configuration file:

```json
{
  "match-type": "glob",
  "match": "*.{step,stp,iges,igs,brep,xbf}",
  "options": {
    "scalar-coloring": false
  }
}
```

### Linux

> I have a link error related to `stdc++fs` not found or I'm unable to run F3D due to filesystem errors.

With some C++ STD library version, explicit linking to `stdc++fs` is not supported or required. Set the `F3D_LINUX_LINK_FILESYSTEM` CMake option to the desired value to workaround this issue.

> I have a link error related to undefined reference to symbol of `libatomic`.

The GCC flag `-latomic` is not added automatically with specific architectures, like `armel` and `RISCV64`. Set the `F3D_LINUX_LIBRARY_LINK_ATOMIC` CMake option to `ON` to workaround this issue.

> Thumbnails are not working in my file manager.

- Check that your file manager supports the thumbnailer mechanism.
- Check that you updated your mime type database.
- If all fails, remove your `.cache` user dir and check that `pcmanfm` thumbnails are working.
- If they are working, the issue is specific to your file manager (see below for a potential work around).
- If only a few format have working thumbnails, the issue is with the mime types database.
- If no formats have working thumbnails, check sandboxing and the `f3d-plugin-xxx.thumbnailer` files.
- If only big file do not have thumbnails, this is intended. Modify this behavior in the `thumbnail.d/05_all.json` configuration directory using the `max-size` option.

> I'm unable to link C++ examples against my local F3D install directory, it complains about missing VTK symbols

Install VTK on your system or add the VTK install lib directory to `LD_LIBRARY_PATH`.

### Windows

> Using the portable binary release, I'm unable to run F3D because Windows warns about a security risk

F3D is not signed by Microsoft yet, so Windows shows this warning. Click on the "more" button to run F3D.

> After installing F3D or registering the shell extension, my explorer is broken.

Unregister the shell extension:

```bat
cd C:\path\to\f3d\bin\
regsvr32 /u F3DShellExtension.dll
```

> I use F3D in a VM, the application fails to launch.

OpenGL applications like F3D can have issues on Windows Server or guest Windows because access to the GPU is restricted.
Use a software implementation of OpenGL, called [Mesa](https://github.com/pal1000/mesa-dist-win/releases).

- Download the latest `release-msvc`.
- Copy `x64/opengl32.dll`, `libgallium_wgl.dll` and `x64/libglapi.dll` in the same directory as `f3d.exe`.
- Set the environment variable `MESA_GL_VERSION_OVERRIDE` to 4.5.
- Run `f3d.exe`.

> I run f3d from the command prompt and my Unicode characters are not displayed properly.

Set the codepage to UTF-8. Run `chcp 65001`.

> When I install F3D, I can't add F3D to the path and the installer complains that: `PATH too long installer unable to modify PATH!`.

Your PATH environment variable has too many entries. Remove the ones you do not use.

> When I try to update F3D, the installer fails with: `Error opening file for writing: F3DShellExtension.dll`

Your previous F3D installation is currently being used to generate thumbnails. Wait for a bit or reboot your computer before updating again.

> When running F3D without interactivity (e.g. `f3d.exe --version`) from CLI, nothing is displayed.

This is a limitation from the Windows "subsystem". Use `f3d-console.exe` instead.

> When playing an animation, the animation appear to move too slowly

This is a limitation of older VTK version before VTK 9.5.0. Use F3D binaries or compile a recent version of VTK.

### macOS

> I'm unable to use the binary release, macOS complains the file is invalid or corrupted

The macOS binary package are not signed. To work around this, run the following command:

```bash
xattr -cr /Applications/F3D.app
```

Then F3D works as expected.

### Python

> I can't find `usd` and `vdb` plugins after installing f3d with pip

The `usd` and `vdb` plugins are not shipped in the Python wheels for now. You can compile F3D yourself with them.

> Raytracing does not work after installing f3d with pip

F3D raytracing feature is not shipped in the Python wheels for now. You can compile F3D yourself with it.

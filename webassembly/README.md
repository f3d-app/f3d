# Introduction

F3D can be built in WebAssembly using emscripten in order to embed it into a web browser.
It is still experimental and only a small fraction of the libf3d public API is bound.
An example can be seen at https://f3d.app/wasm

# Building

While it's possible to setup an emscripten cross-compiling toolchain locally, it's easier to use Docker to build it

## Preparing the build

Install Docker locally and pull the WebAssembly image

```sh
docker pull dockcross/web-wasm:20230905-7b2d74f
```

Clone VTK and F3D, it will be assumed that the source code is located in `$VTK_DIR` and `$F3D_DIR` variables in the next steps of this guide.

## Building VTK

Configure VTK by running the following command:

```sh
docker run -v $VTK_DIR:/vtk -rm dockcross/web-wasm:20230905-7b2d74f cmake -DBUILD_SHARED_LIBS=OFF -DCMAKE_BUILD_TYPE=Release -DVTK_ENABLE_LOGGING=OFF -DVTK_ENABLE_WRAPPING=OFF -DVTK_MODULE_ENABLE_VTK_RenderingContext2D=YES -DVTK_MODULE_ENABLE_VTK_InteractionWidgets=YES -DVTK_MODULE_ENABLE_VTK_RenderingLICOpenGL2=NO -DVTK_MODULE_ENABLE_VTK_hdf5=NO -DVTK_MODULE_ENABLE_VTK_libproj=NO -S /vtk -B /vtk/build-wasm
```

Build VTK by running the following command:

```sh
docker run -v $VTK_DIR:/vtk --rm dockcross/web-wasm:20230905-7b2d74f cmake --build /vtk/build-wasm --parallel 8
```

## Building F3D

Configure F3D by running the following command:

```sh
docker run -v $VTK_DIR:/vtk -v $F3D_DIR:/f3d --rm dockcross/web-wasm:20230905-7b2d74f cmake -DBUILD_SHARED_LIBS=OFF -DCMAKE_BUILD_TYPE=Release -DVTK_DIR=/vtk/build-wasm  -DF3D_PLUGIN_BUILD_EXODUS=OFF -DF3D_WASM_DATA_FILE=/f3d/testing/data/f3d.vtp -S /f3d -B /f3d/build-wasm
```

> You can change the value of `F3D_WASM_DATA_FILE` to embed another file in the virtual filesystem.

Build F3D by running the following command:

```sh
docker run -v $VTK_DIR:/vtk -v $F3D_DIR:/f3d --rm dockcross/web-wasm:20230905-7b2d74f cmake --build /f3d/build-wasm --parallel 8
```

# Testing it locally

Five different files should be located in `$F3D_DIR/build-wasm/bin` folder:
- `f3d.js`: javascript library
- `f3d.wasm`: webassembly binaries
- `f3d.data`: virtual filesystem containing the file specified in `F3D_WASM_DATA_FILE`
- `index.html`: example file setting up the web page
- `favicon.ico`: icon used to display the logo in the browser tab

It's possible to run a local HTML server by running the following command from `$F3D_DIR/build-wasm/bin`:

```sh
python -m http.server 8000
```

Going to http://localhost:8000 should display the web page and load F3D.

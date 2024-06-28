# Introduction

F3D can be built in WebAssembly using emscripten in order to embed it into a web browser.
It is still experimental and only a small fraction of the libf3d public API is bound.
An example can be seen at https://f3d.app/web

This guide is describing how to build VTK and F3D with emscripten using Docker on Linux or Windows.

# Building

While it's possible to setup an emscripten cross-compiling toolchain locally, it's easier to use Docker to build it

## Preparing the build

Install Docker locally and pull the WebAssembly image

```sh
docker pull dockcross/web-wasm:20240529-0dade71
```

Clone VTK and F3D, it will be assumed that the source code is located in `$VTK_DIR` and `$F3D_DIR` variables in the next steps of this guide.
It's recommended to use VTK commit `7e954a92fba020b160a07420403248f6be87f2b0` which has been tested to work properly.

## Building VTK

Configure VTK by running the following command:

```sh
docker run -v ${VTK_DIR}:/vtk \
 --rm dockcross/web-wasm:20240529-0dade71 \
 cmake -S /vtk -B /vtk/build-wasm \
 -DBUILD_SHARED_LIBS=OFF \
 -DCMAKE_BUILD_TYPE=Release \
 -DVTK_ENABLE_LOGGING=OFF \
 -DVTK_ENABLE_WRAPPING=OFF \
 -DVTK_GROUP_ENABLE_Imaging=DONT_WANT \
 -DVTK_GROUP_ENABLE_MPI=DONT_WANT \
 -DVTK_GROUP_ENABLE_Qt=DONT_WANT \
 -DVTK_GROUP_ENABLE_Rendering=DONT_WANT \
 -DVTK_GROUP_ENABLE_StandAlone=DONT_WANT \
 -DVTK_GROUP_ENABLE_Views=DONT_WANT \
 -DVTK_GROUP_ENABLE_Web=DONT_WANT \
 -DVTK_MODULE_ENABLE_VTK_CommonCore=YES \
 -DVTK_MODULE_ENABLE_VTK_CommonDataModel=YES \
 -DVTK_MODULE_ENABLE_VTK_CommonExecutionModel=YES \
 -DVTK_MODULE_ENABLE_VTK_CommonSystem=YES \
 -DVTK_MODULE_ENABLE_VTK_FiltersGeneral=YES \
 -DVTK_MODULE_ENABLE_VTK_FiltersGeometry=YES \
 -DVTK_MODULE_ENABLE_VTK_IOCityGML=YES \
 -DVTK_MODULE_ENABLE_VTK_IOImage=YES \
 -DVTK_MODULE_ENABLE_VTK_IOImport=YES \
 -DVTK_MODULE_ENABLE_VTK_IOPLY=YES \
 -DVTK_MODULE_ENABLE_VTK_IOParallel=YES \
 -DVTK_MODULE_ENABLE_VTK_IOXML=YES \
 -DVTK_MODULE_ENABLE_VTK_ImagingCore=YES \
 -DVTK_MODULE_ENABLE_VTK_ImagingHybrid=YES \
 -DVTK_MODULE_ENABLE_VTK_InteractionStyle=YES \
 -DVTK_MODULE_ENABLE_VTK_InteractionWidgets=YES \
 -DVTK_MODULE_ENABLE_VTK_RenderingAnnotation=YES \
 -DVTK_MODULE_ENABLE_VTK_RenderingCore=YES \
 -DVTK_MODULE_ENABLE_VTK_RenderingOpenGL2=YES \
 -DVTK_MODULE_ENABLE_VTK_RenderingVolumeOpenGL2=YES \
 -DVTK_MODULE_ENABLE_VTK_TestingCore=YES \
 -DVTK_LEGACY_REMOVE=ON \
 -DVTK_SMP_IMPLEMENTATION_TYPE=Sequential
```

Build VTK by running the following command:

```sh
docker run -v ${VTK_DIR}:/vtk --rm dockcross/web-wasm:20240529-0dade71 cmake --build /vtk/build-wasm --parallel 8
```

## Building F3D

Configure F3D by running the following command:

```sh
docker run -v ${VTK_DIR}:/vtk -v ${F3D_DIR}:/f3d \
 --rm dockcross/web-wasm:20240529-0dade71 \
 cmake -S /f3d -B /f3d/build-wasm \
 -DBUILD_SHARED_LIBS=OFF \
 -DCMAKE_BUILD_TYPE=Release \
 -DVTK_DIR=/vtk/build-wasm \
 -DF3D_PLUGIN_BUILD_EXODUS=OFF \
 -DF3D_WASM_DATA_FILE="/f3d/testing/data/f3d.vtp"
```

> You can change the value of `F3D_WASM_DATA_FILE` to embed another file in the virtual filesystem.

Build F3D by running the following command:

```sh
docker run -v ${VTK_DIR}:/vtk -v ${F3D_DIR}:/f3d --rm dockcross/web-wasm:20240529-0dade71 cmake --build /f3d/build-wasm --parallel 8
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

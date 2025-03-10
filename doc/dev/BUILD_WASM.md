# Introduction

F3D can be built in WebAssembly using emscripten in order to embed it into a web browser.
It is still experimental and only a small fraction of the libf3d public API is bound.
An example can be seen at https://f3d.app/web

This guide is describing how to build VTK and F3D with emscripten using Docker on Linux or Windows.

# Building

While it's possible to setup an emscripten cross-compiling toolchain locally, it's easier to use Docker and the provided npm scripts.

## Preparing the build

Install `Docker` and `npm` locally.

## Building F3D

Go to the `webassembly` folder and run the following command to build F3D:

```sh
npm run build:deb # or build:rel for optimized build
```

On completion, a folder `webassembly/dist` is created containing the artifacts.

# Testing it locally

Five different files should be located in `webassembly/dist` folder:

- `f3d.js`: javascript library
- `f3d.wasm`: webassembly binaries
- `f3d.data`: virtual filesystem containing the file specified in `F3D_WASM_DATA_FILE`
- `index.html`: example file setting up the web page
- `favicon.ico`: icon used to display the logo in the browser tab

Again, use a npm command to test locally by running a HTTP server:

```sh
npm run serve
```

Going to http://localhost:3000 should display the web page and load F3D web app.

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

From the root of the repository run the following command:

```sh
npm run build
```

On completion, a directory `dist` is created containing the artifacts.

# Run tests

From the root of the repository, after the build step, run the following command:

```sh
npm test
```


# Integrating

It's possible to generate a local package to use in other javascript projects.
Run the following command:

```sh
npm pack
```

It will build and generate

Five different files should be located in `webassembly/dist` directory:

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

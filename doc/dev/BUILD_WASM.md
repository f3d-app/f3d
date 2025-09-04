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

It will build and generate a `f3d-vX.X.X.tgz` file.
This file can be imported into your project:

```sh
npm install f3d-vX.X.X.tgz
```

# Build WebAssembly

F3D can be built in WebAssembly using emscripten in order to embed it into a web browser.
It is used by our [web viewer](https://f3d.app/viewer) and covers most of the [libf3d public API](../libf3d/04-LANGUAGE_BINDINGS.md).
An simple example is available [here](https://github.com/f3d-app/f3d/blob/master/examples/libf3d/web).

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

By default, the latest Docker image is pulled automatically, but you can force a specific Docker image by setting the environment variable `F3D_DOCKER_TIMESTAMP` to a timestamp (see `.github/workflows/versions.json` file)
It's also possible to use `podman` instead of `docker`, please set the `F3D_DOCKER_CLIENT` environment variable to `podman`.
On completion, a directory `dist` is created containing the artifacts (`f3d.js` and `f3d.wasm`).
`assimp`, `occt` and `draco` plugins are automatically included.

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

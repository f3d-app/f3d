F3D is a fast and minimalist 3D viewer. Implemented in C++, it has been cross-compiled with emscripten to a WebAssembly module.

While the webassembly module might be quite large (~14MB), it is self contained and requires no external dependency.

# Installation

Install `f3d` package. Using `npm`, running this command in your app is enough:

```bash
npm install f3d
```

# Usage

Create the target DOM element `<canvas id="canvas"></canvas>`, then use the following javascript code:

```javascript
// import the module
import f3d from "f3d";

// setup a setting object
const settings = {
  canvas: document.getElementById("canvas"),
  setupOptions: (options) => {
    // background must be set to black for proper blending with transparent canvas
    options.set_color("render.background.color", 0, 0, 0);

    // make it look nice
    options.toggle("render.effect.antialiasing.enable");
    options.toggle("render.effect.tone_mapping");
    options.toggle("render.effect.ambient_occlusion");
    options.toggle("render.hdri.ambient");

    // display widgets
    options.toggle("ui.axis");
    options.toggle("render.grid.enable");
  },
};

f3d(settings)
  .then(async (Module) => {
    // write a 3D file located on the server to the internal filesystem
    const modelFile = await fetch("/assets/example.obj").then((b) => b.arrayBuffer());
    Module.FS.writeFile("example.obj", new Uint8Array(modelFile));

    // automatically load all supported file format readers
    Module.Engine.autoloadPlugins();

    // create an engine
    Module.engineInstance = Module.Engine.create();

    Module.setupOptions(Module.engineInstance.getOptions());

    // setup the window size based on the canvas size
    const scale = window.devicePixelRatio;
    Module.engineInstance
      .getWindow()
      .setSize(Module.canvas.clientWidth, scale * Module.canvas.clientHeight);

    const scene = Module.engineInstance.getScene();
    scene.add("/example.obj");

    // do a first render and start the interactor
    Module.engineInstance.getWindow().render();
    Module.engineInstance.getInteractor().start();
  })
  .catch((error) => {
    console.error("Internal exception: " + error);
  });
```

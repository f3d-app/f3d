import f3d from "f3d";

f3d({})
  .then(async (Module) => {
    // automatically load all supported file format readers
    Module.Engine.autoloadPlugins();

    const engine = Module.Engine.create();

    // background must be set to black for proper blending with transparent canvas
    engine.getOptions().setAsString("render.background.color", "black");
    engine.getOptions().setAsString("scene.up_direction", "-y");
    engine.getOptions().toggle("model.scivis.enable");
    engine.getOptions().setAsString("model.scivis.component", "-2");
    engine.getOptions().setAsString("model.point_sprites.type", "gaussian");
    engine.getOptions().setAsString("model.point_sprites.size", "1.0");
    engine.getOptions().toggle("model.point_sprites.absolute_size");
    engine
      .getOptions()
      .setAsString("render.effect.blending.mode", "stochastic");
    engine.getOptions().setAsString("render.effect.antialiasing.mode", "taa");

    // setup the window size based on the canvas size
    const canvas = document.getElementById("canvas");
    const scale = window.devicePixelRatio;
    engine
      .getWindow()
      .setSize(scale * canvas.clientWidth, scale * canvas.clientHeight);

    // read file and display it
    const response = await fetch(`https://f3d.app/data/cluster_fly.spz`);
    const arrayBuffer = await response.arrayBuffer();
    const scene = engine.getScene();
    try {
      scene.addBuffer(new Uint8Array(arrayBuffer));
    } catch (e) {
      console.error("Unsupported file");
    }

    // do a first render and start the interactor
    engine.getWindow().render();
    engine.getInteractor().start();
  })
  .catch((error) => console.error("Internal exception: " + error));

import f3d from "f3d";

f3d({})
  .then(async (Module) => {
    Module.Log.setVerboseLevel(Module.LogVerboseLevel.QUIET, false);
    Module.Log.forward((level, message) => {
      if (level === Module.LogVerboseLevel.ERROR) console.error(message);
      else if (level === Module.LogVerboseLevel.WARN) console.warn(message);
      else if (level === Module.LogVerboseLevel.INFO) console.info(message);
    });

    // automatically load all supported file format readers
    Module.Engine.autoloadPlugins();

    for (const { id, file } of [
      { id: "canvas1", file: "DamagedHelmet.glb" },
      { id: "canvas2", file: "202.vtp" },
    ]) {
      const engine = Module.Engine.create("#" + id);

      // background must be set to black for proper blending with transparent canvas
      engine.getOptions().setAsString("render.background.color", "#000000");

      // make it look nice
      engine
        .getOptions()
        .setAsString("render.effect.antialiasing.mode", "fxaa");
      engine.getOptions().toggle("render.effect.tone_mapping");
      engine.getOptions().toggle("render.effect.ambient_occlusion");
      engine.getOptions().toggle("render.hdri.ambient");

      // setup the window size based on the canvas size
      const canvas = document.getElementById(id);
      const scale = window.devicePixelRatio;
      engine
        .getWindow()
        .setSize(scale * canvas.clientWidth, scale * canvas.clientHeight);

      // read https://f3d.app/data/{file} and display it
      const response = await fetch(`https://f3d.app/data/${file}`);
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
    }
  })
  .catch((error) => console.error("Internal exception: " + error));

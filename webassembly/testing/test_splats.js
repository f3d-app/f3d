import utils from "./utils.js";

const settings = {
  runBefore: (Module) => {
    const options = Module.engineInstance.getOptions();

    // background must be set to black for proper blending with transparent canvas
    options.setAsString("render.background.color", "#000000");

    // setup coloring
    options.toggle("model.scivis.enable");
    options.setAsString("model.scivis.component", "-2");

    // splats
    options.toggle("model.point_sprites.enable");
    options.toggle("model.point_sprites.absolute_size");
    options.setAsString("model.point_sprites.type", "gaussian");
    options.toggle("render.effect.blending.enable");
    options.setAsString("render.effect.blending.mode", "stochastic");

    // display widgets
    options.toggle("ui.axis");

    // default to +Z
    options.setAsString("scene.up_direction", "-Y");
  },

  runAfter: (Module) => {
    const cam = Module.engineInstance.getWindow().getCamera();
    cam.dolly(3.0);
  },
};

utils.runRenderTest(settings, {
  data: "small.splat",
  baseline: "TestWasmSplats.png",
});

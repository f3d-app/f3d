import utils from "./utils.js";

const settings = {
  runBefore: (Module) => {
    const options = Module.engineInstance.getOptions();

    // background must be set to black for proper blending with transparent canvas
    options.setAsString("render.background.color", "#000000");

    // setup coloring
    options.toggle("model.scivis.enable");
    options.toggle("render.effect.display_depth");

    // default to +Z
    options.setAsString("scene.up_direction", "+Z");
  },
};

utils.runRenderTest(settings, {
  data: "f3d.vtp",
  baseline: "TestWasmDepth.png",
});

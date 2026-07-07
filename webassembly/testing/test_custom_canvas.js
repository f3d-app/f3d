import utils from "./utils.js";

const settings = {
  runBefore: (Module) => {
    const options = Module.engineInstance.getOptions();

    // background must be set to black for proper blending with transparent canvas
    options.setAsString("render.background.color", "#000000");

    // setup coloring
    options.toggle("model.scivis.enable");
    options.setAsString("model.scivis.array_name", "Colors");
    options.setAsString("model.scivis.component", "-2");
    options.toggle("model.scivis.cells");

    // default to +Z
    options.setAsString("scene.up_direction", "+Z");
  },
  canvasId: "custom_id",
};

utils.runRenderTest(settings, {
  data: "f3d.vtp",
  baseline: "TestWasmCustomCanvas.png",
});

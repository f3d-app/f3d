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

    // make it look nice
    options.toggle("render.effect.antialiasing.enable");
    options.toggle("render.effect.tone_mapping");
    options.toggle("render.effect.ambient_occlusion");
    options.toggle("render.hdri.ambient");

    // display widgets
    options.toggle("ui.axis");
    options.toggle("render.grid.enable");

    // default to +Z
    options.setAsString("scene.up_direction", "+Z");
  },

  runAfter: (Module) => {
    // check that space conversion works
    const window = Module.engineInstance.getWindow();

    const ptWorld = window.getWorldFromDisplay([0, 0, 0]);

    utils.assert(
      !utils.numArrayEquals(ptWorld, [0, 0, 0], 0.001),
      "point has no been transformed",
    );

    const ptDisplay = window.getDisplayFromWorld(ptWorld);

    utils.assert(
      utils.numArrayEquals(ptDisplay, [0, 0, 0], 0.001),
      "point has no been restored to original value",
    );

    // just for coverage
    Module.engineInstance.setCachePath("/tmp");
  },
};

utils.runRenderTest(settings, {
  data: "f3d.vtp",
  baseline: "TestWasmOptions.png",
});

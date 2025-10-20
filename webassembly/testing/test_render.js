import utils from "./utils.js";

const settings = {
  setupOptions: (options) => {
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
};

utils.runRenderTest(settings, "f3d.vtp", "TestWasmOptions.png");

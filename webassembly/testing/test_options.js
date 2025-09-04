import utils from "./utils.js";

const settings = {
  setupOptions: (options) => {
    // background must be set to black for proper blending with transparent canvas
    options.set_color("render.background.color", 0, 0, 0);

    // setup coloring
    options.toggle("model.scivis.enable");
    options.set_string("model.scivis.array_name", "Colors");
    options.set_integer("model.scivis.component", -2);
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
    options.set_as_string("scene.up_direction", "+Z");
  },
  dataPath: "f3d.vtp",
  baselinePath: "TestWasmOptions.png",
};

utils.runTest(settings);

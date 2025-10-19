import utils from "./utils.js";

const settings = {
  setupOptions: (options) => {
    // background must be set to black for proper blending with transparent canvas
    options.setAsString("render.background.color", "#000000");

    // setup coloring
    options.toggle("model.scivis.enable");
    options.setAsString("model.scivis.array_name", "Colors");
    options.setAsString("model.scivis.component", "-2");
    options.setAsString("model.scivis.range", "0.7,1.4"); // not used for the baseline, only for coverage
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

    // check that the getter works
    utils.assert(options.get("model.scivis.enable") === true, "options getter for boolean failed");
    utils.assert(options.get("model.scivis.component") === -2, "options getter for int failed");
    utils.assert(options.get("model.point_sprites.size") === 10.0, "options getter for double failed");
    utils.assert(options.get("model.point_sprites.type") === "sphere", "options getter for string failed");

    // comparing JS arrays
    utils.assert(JSON.stringify(options.get("model.scivis.range")) === JSON.stringify([0.7,1.4]), "options getter for vec<double> failed");
  },
  dataPath: "f3d.vtp",
  baselinePath: "TestWasmOptions.png",
};

utils.runTest(settings);

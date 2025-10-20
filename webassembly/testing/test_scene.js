import utils from "./utils.js";

const settings = {
  runBefore: (Module) => {

    // does nothing but called for coverage
    Module.engineInstance.getScene().clear();

    const options = Module.engineInstance.getOptions();

    // background must be set to black for proper blending with transparent canvas
    options.setAsString("render.background.color", "#000000");

    // display widgets
    options.toggle("ui.axis");
    options.toggle("render.grid.enable");
  },

  runAfter: (Module) => {

    const scene = Module.engineInstance.getScene();

    utils.assert(scene.availableAnimations() == 1, "There should be a single animation");

    const [ start, end ] = scene.animationTimeRange();

    utils.assert(start === 0, "Start value should be 0");
    utils.assert(end === 4, "End value should be 4");

    scene.loadAnimationTime(1.5);
  },
};

utils.runRenderTest(settings, { data: "f3d.glb", baseline: "TestWasmAnimation.png" });

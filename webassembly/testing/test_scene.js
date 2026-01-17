import utils from "./utils.js";

const settings = {
  runBefore: (Module) => {
    // does nothing but called for coverage
    Module.engineInstance.getScene().addBuffer(new Array());
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

    utils.assert(
      scene.availableAnimations() == 1,
      "There should be a single animation",
    );

    const [start, end] = scene.animationTimeRange();

    utils.assert(start === 0, "Start value should be 0");
    utils.assert(end === 4, "End value should be 4");

    utils.assert(
      scene.animationKeyFrames().length === 50,
      "KeyFrames length should be 50",
    );
    utils.assert(
      scene.animationKeyFrames()[0] === 0,
      "First KeyFrame should be 0",
    );
    utils.assert(
      scene.animationKeyFrames()[49] === 4.0,
      "First KeyFrame should be 4.0",
    );

    scene.loadAnimationTime(1.5);

    utils.assert(
      scene.getAnimationName() == "unnamed_0",
      "getAnimationName returns name",
    );

    // array comparison in JS is a little annoying so we just compare the 0th element
    utils.assert(
      scene.getAnimationNames()[0] == "unnamed_0",
      "getAnimationNames returns names",
    );
  },
};

utils.runRenderTest(settings, {
  data: "f3d.glb",
  baseline: "TestWasmAnimation.png",
});

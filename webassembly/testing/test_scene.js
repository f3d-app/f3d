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
      scene.availableAnimations() == 10,
      "There should be a single animation",
    );

    const [start, end] = scene.animationTimeRange();

    utils.assert(start === 0, "Start value should be 0");
    utils.assert(
      end === 0.7999999999999999,
      "End value should be 0.7999999999999999",
    );

    utils.assert(
      scene.getAnimationKeyFrames().length === 9,
      "KeyFrames length should be 9",
    );
    utils.assert(
      scene.getAnimationKeyFrames()[0] === 0,
      "First KeyFrame should be 0",
    );
    utils.assert(
      scene.getAnimationKeyFrames()[8] === 0.7999999999999999,
      "First KeyFrame should be 0.7999999999999999",
    );

    scene.loadAnimationTime(0.5);

    utils.assert(
      scene.getAnimationName() == "stand",
      "getAnimationName returns name",
    );

    // array comparison in JS is a little annoying so we just compare the 0th element
    utils.assert(
      scene.getAnimationNames()[0] == "stand",
      "getAnimationNames returns names",
    );
  },
};

utils.runRenderTest(settings, {
  data: "soldier_animations.mdl",
  baseline: "TestWasmAnimation.png",
});

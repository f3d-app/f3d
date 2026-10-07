import utils from "./utils.js";

const settings = {
  runAfter: (Module) => {
    const anim = Module.engineInstance.getScene().getAnimation();

    utils.assert(anim.count() == 10, "There should be a single animation");

    const [start, end] = anim.getTimeRange();

    utils.assert(start === 0, "Start value should be 0");
    utils.assert(
      end === 0.7999999999999999,
      "End value should be 0.7999999999999999",
    );

    utils.assert(anim.getKeyFrames().length === 9, "KeyFrames length should be 9");
    utils.assert(anim.getKeyFrames()[0] === 0, "First KeyFrame should be 0");
    utils.assert(
      anim.keyFrames()[8] === 0.7999999999999999,
      "First KeyFrame should be 0.7999999999999999",
    );

    anim.loadTime(0.5);

    utils.assert(anim.getName() == "stand", "getAnimationName returns name");
    utils.assert(anim.getName(1) == "dead", "getAnimationName returns name");

    // array comparison in JS is a little annoying so we just compare the 0th element
    utils.assert(
      anim.getNames()[0] == "stand",
      "getAnimationNames returns names",
    );
  },
};

utils.runRenderTest(settings, {
  data: "soldier_animations.mdl",
  baseline: "TestWasmAnimation.png",
});

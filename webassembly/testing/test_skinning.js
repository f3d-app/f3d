import utils from "./utils.js";

const settings = {
  runBefore: (Module) => {
    const options = Module.engineInstance.getOptions();

    // background must be set to black for proper blending with transparent canvas
    options.setAsString("render.background.color", "#000000");

    // armature
    options.toggle("render.armature.enable");
  },

  runAfter: (Module) => {
    Module.engineInstance.getScene().loadAnimationTime(0.5);
  },
};

utils.runRenderTest(settings, {
  data: "RiggedFigure.glb",
  baseline: "TestWasmSkinning.png",
});

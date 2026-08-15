import utils from "./utils.js";

const settings = {
  runBefore: (Module) => {
    // does nothing but called for coverage
    Module.engineInstance.getScene().addBuffer(new Array());
    Module.engineInstance.getScene().clear();

    utils.assert(
      Module.engineInstance.getScene().getSceneHierarchy().length === 0,
      "a cleared scene should have an empty scene hierarchy",
    );

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
      scene.getAnimationName(-1) == "stand",
      "getAnimationName returns name",
    );

    // array comparison in JS is a little annoying so we just compare the 0th element
    utils.assert(
      scene.getAnimationNames()[0] == "stand",
      "getAnimationNames returns names",
    );

    const hierarchy = scene.getSceneHierarchy();

    utils.assert(hierarchy.length > 0, "scene hierarchy should not be empty");
    utils.assert(
      hierarchy[0].parentId === -1 && hierarchy[0].hasChildren,
      "root node should have no parent and have children",
    );
    utils.assert(
      hierarchy[0].label === "soldier_animations.mdl",
      "root node should be labeled after the added file",
    );
    utils.assert(
      hierarchy.every((node, index) => node.id === index),
      "node ids should be their index in the hierarchy",
    );
    utils.assert(
      hierarchy.every(
        (node) =>
          node.parentId < node.id &&
          node.level ===
            (node.parentId < 0 ? 0 : hierarchy[node.parentId].level + 1),
      ),
      "a parent node should precede its children and be one level above them",
    );
    utils.assert(
      hierarchy.every((node) => node.visible),
      "nodes should be visible by default",
    );

    scene.setNodeVisibility(0, false);
    utils.assert(
      scene.getSceneHierarchy().every((node) => !node.visible),
      "hiding the root node should hide its whole subtree",
    );

    // restore the visibility expected by the baseline
    scene.setNodeVisibility(0, true);
    utils.assert(
      scene.getSceneHierarchy().every((node) => node.visible),
      "showing the root node should show its whole subtree",
    );
  },
};

utils.runRenderTest(settings, {
  data: "soldier_animations.mdl",
  baseline: "TestWasmAnimation.png",
});

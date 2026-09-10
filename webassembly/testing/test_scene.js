import utils from "./utils.js";

const settings = {
  runBefore: (Module) => {
    const scene = Module.engineInstance.getScene();

    // does nothing but called for coverage
    scene.addBuffer(new Array());
    scene.clear();

    utils.assert(
      scene.getSceneHierarchy().length === 0,
      "a cleared scene should have an empty scene hierarchy",
    );

    const clearedInfo = Module.engineInstance.getScene().getSceneInfo();
    utils.assert(
      clearedInfo.numberOfFiles === 0 &&
        clearedInfo.numberOfActors === 0 &&
        clearedInfo.numberOfPoints === 0n &&
        clearedInfo.numberOfCells === 0n,
      "a cleared scene should have zeroed scene info",
    );

    utils.assert(
      scene.supports("unsupportedFile.dummy") ===
        Module.FileAvailability.UNSUPPORTED_EXTENSION,
      "an unknown extension should be reported as unsupported",
    );

    utils.assert(
      scene.supports("invalid.mdl") ===
        Module.FileAvailability.UNSUPPORTED_CONTENT,
      "a file with an invalid header should be reported as unsupported content",
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
      scene.animationTime() === 0.5,
      "animationTime should be 0.5 after loading",
    );

    utils.assert(
      scene.animationFrame() === 15,
      "animationFrame should be 15 after loading 0.5",
    );

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

    const info = scene.getSceneInfo();

    utils.assert(
      info.numberOfFiles === 1 && info.numberOfActors > 0,
      "scene info should count the added file and its actors",
    );
    utils.assert(
      typeof info.numberOfPoints === "bigint" &&
        typeof info.numberOfCells === "bigint",
      "64 bits scene info counters should be mapped to BigInt",
    );
    utils.assert(
      info.numberOfPoints > 0n && info.numberOfCells > 0n,
      "scene info should count points and cells",
    );

    // add a custom mesh
    let mesh = new Module.Mesh();
    mesh.points = new Float32Array([0, 0, 0, 30, 0, 0, 0, 30, 0]);
    mesh.normals = new Float32Array([0, 0, 1, 0, 0, 1, 0, 0, 1]);
    mesh.textureCoordinates = new Float32Array([0, 0, 1, 0, 0, 1]);
    mesh.faceSides = new Uint32Array([3]);
    mesh.faceIndices = new Uint32Array([0, 1, 2]);
    scene.addMesh(mesh);

    const newInfo = scene.getSceneInfo();

    utils.assert(
      newInfo.numberOfFiles === 2 && newInfo.numberOfActors > 0,
      "scene info should count the added file and its actors",
    );

    // check lights
    utils.assert(scene.getLightCount() === 5, "scene should have five lights");

    let light0 = scene.getLight(0);
    utils.assert(
      light0.type === Module.LightType.HEADLIGHT,
      "first light should be headlight",
    );

    // modify first light color / intensity
    light0.color = new Module.Color(1.0, 0.0, 0.0);
    light0.intensity *= 3.0;
    scene.updateLight(0, light0);

    // remove last light
    scene.removeLight(4);
    utils.assert(scene.getLightCount() === 4, "scene should have four lights");

    // remove all lights
    scene.removeAllLights();
    utils.assert(scene.getLightCount() === 0, "scene should have no lights");

    // restore first headlight
    scene.addLight(light0);
    utils.assert(scene.getLightCount() === 1, "scene should have one light");
  },
};

utils.runRenderTest(settings, {
  data: "soldier_animations.mdl",
  extraData: ["invalid.mdl", "unsupportedFile.dummy"],
  baseline: "TestWasmAnimation.png",
});

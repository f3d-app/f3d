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
    const interactor = Module.engineInstance.getInteractor();

    // commands
    interactor.initCommands();

    let callbackCalled = false;

    interactor.addCommand("foo", (args) => {
      utils.assert(args[0] === "bar");
      callbackCalled = true;
    });

    utils.assert(
      interactor.getCommandActions().includes("foo"),
      "command not added",
    );

    interactor.triggerCommand("foo bar baz", true);

    utils.assert(callbackCalled, "command callback not called");

    interactor.removeCommand("foo");
    utils.assert(
      !interactor.getCommandActions().includes("foo"),
      "command not removed",
    );

    // animations
    utils.assert(
      !interactor.isPlayingAnimation(),
      "animation should not be playing",
    );

    interactor.startAnimation();
    utils.assert(
      interactor.isPlayingAnimation(),
      "animation should be playing",
    );

    interactor.stopAnimation();
    utils.assert(
      !interactor.isPlayingAnimation(),
      "animation should not be playing",
    );

    interactor.toggleAnimation();
    utils.assert(
      interactor.isPlayingAnimation(),
      "animation should be playing",
    );

    // only for coverage, do not test the actual feature yet
    interactor.disableCameraMovement();
    interactor.enableCameraMovement();
    interactor.requestRender();
    interactor.requestStop();
    interactor.stop();
  },
};

utils.runRenderTest(settings, {
  data: "f3d.glb",
  baseline: "TestWasmInteraction.png",
});

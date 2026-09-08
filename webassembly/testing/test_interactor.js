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

    interactor.startAnimation(Module.InteractorAnimationDirection.FORWARD);
    utils.assert(
      interactor.isPlayingAnimation() &&
        interactor.getAnimationDirection() ==
          Module.InteractorAnimationDirection.FORWARD,
      "animation should be playing forward",
    );
    interactor.stopAnimation();
    utils.assert(
      !interactor.isPlayingAnimation(),
      "animation should not be playing",
    );

    interactor.startAnimation(Module.InteractorAnimationDirection.BACKWARD);
    utils.assert(
      interactor.isPlayingAnimation() &&
        interactor.getAnimationDirection() ==
          Module.InteractorAnimationDirection.BACKWARD,
      "animation should be playing backward",
    );
    interactor.stopAnimation();

    interactor.toggleAnimation(Module.InteractorAnimationDirection.FORWARD);
    utils.assert(
      interactor.isPlayingAnimation() &&
        interactor.getAnimationDirection() ==
          Module.InteractorAnimationDirection.FORWARD,
      "animation should be playing forward",
    );
    interactor.stopAnimation();

    interactor.toggleAnimation(Module.InteractorAnimationDirection.BACKWARD);
    utils.assert(
      interactor.isPlayingAnimation() &&
        interactor.getAnimationDirection() ==
          Module.InteractorAnimationDirection.BACKWARD,
      "animation should be playing backward",
    );
    interactor.stopAnimation();

    // bindings
    interactor.initBindings();

    const initialBinds = interactor.getBinds();
    utils.assert(
      Array.isArray(initialBinds) && initialBinds.length > 0,
      "bindings should be returned",
    );

    const bind = new Module.InteractionBind();
    bind.mod = Module.InteractionBindModifierKeys.NONE;
    bind.inter = "R";

    interactor.addBinding(
      bind,
      ["set render.grid.reflection .8"],
      "test-group",
      () => ["foo", "bar"],
      Module.InteractorBindingType.OTHER,
      false,
    );

    const updatedBinds = interactor.getBinds();
    utils.assert(
      updatedBinds.length === initialBinds.length + 1,
      "new binding should be registered",
    );

    const bindGroups = interactor.getBindGroups();
    utils.assert(
      bindGroups.includes("test-group"),
      "new bind group should be registered",
    );

    const docs = interactor.getBindingDocumentation(bind);
    utils.assert(
      Array.isArray(docs) && docs[0] === "foo" && docs[1] === "bar",
      "binding documentation should be returned",
    );

    const bindingType = interactor.getBindingType(bind);
    utils.assert(
      bindingType === Module.InteractorBindingType.OTHER,
      "binding type should round-trip",
    );

    // start recording
    interactor.recordInteraction("/test_interaction.txt");

    // input events coverage, just check we do not throw
    // all these calls have no effect on the resulting image
    let inputEventError = false;
    try {
      interactor.triggerModUpdate(Module.InteractionBindModifierKeys.NONE);
      interactor.triggerMousePosition(0, 0);
      interactor.triggerMouseButton(
        Module.InteractorInputAction.PRESS,
        Module.InteractorMouseButton.LEFT,
      );
      interactor.triggerMouseButton(
        Module.InteractorInputAction.RELEASE,
        Module.InteractorMouseButton.LEFT,
      );
      interactor.triggerTextCharacter(0x42);
    } catch {
      inputEventError = true;
    }
    utils.assert(!inputEventError, "input event bindings should not throw");

    // check file content
    const interactionFileContent = Module.FS.readFile("/test_interaction.txt", {
      encoding: "utf8",
    });
    utils.assert(
      interactionFileContent.includes("MouseMoveEvent 0 299 0 0 0 0 0"),
      "interaction file should contain the expected command",
    );

    // play it (no effect on the resulting image, just check we do not throw)
    interactor.playInteraction("/test_interaction.txt", 0.1);

    // zoom out
    interactor.triggerMouseWheel(Module.InteractorWheelDirection.BACKWARD);

    // trigger "set render.grid.reflection .8"
    interactor.triggerKeyboardKey(Module.InteractorInputAction.PRESS, "R");
    interactor.triggerKeyboardKey(Module.InteractorInputAction.RELEASE, "R");

    let bindingRemoved = false;
    try {
      interactor.removeBinding(bind);
    } catch {
      bindingRemoved = true;
    }
    utils.assert(!bindingRemoved, "binding should be removable");

    // notifications
    let notifCount = 0;

    interactor.setNotificationCallback((desc, value, bind, duration) => {
      notifCount++;
      return true;
    });

    interactor.triggerNotification("Test notification", "value", 1.0);
    utils.assert(notifCount === 1, "notification callback not called");

    let eventLoopState = null;
    interactor.setEventLoopUserCallback((state) => {
      eventLoopState = state;
    });
    interactor.triggerEventLoop(0.1);
    utils.assert(
      eventLoopState && typeof eventLoopState.animationTime === "number",
      "event loop state should expose animation time",
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

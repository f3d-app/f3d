import utils from "./utils.js";

const settings = {
  runBefore: (Module) => {

    const options = Module.engineInstance.getOptions();

    // background must be set to black for proper blending with transparent canvas
    options.setAsString("render.background.color", "black");

    // display widgets
    options.toggle("ui.axis");
    options.toggle("render.grid.enable");
  },

  runAfter: (Module) => {

    const camera = Module.engineInstance.getWindow().getCamera();

    utils.assert(utils.numArrayEquals(camera.position, [0, -0.016, 7.059], 0.01));
    utils.assert(utils.numArrayEquals(camera.focalPoint, [0, -0.016, 0.022], 0.01));
    utils.assert(utils.numArrayEquals(camera.viewUp, [0, 1, 0], 0.01));
    utils.assert(Math.abs(camera.viewAngle - 26.444) < 0.01);

    camera.position = [0, 5, 0];
    camera.focalPoint = [0, 0, 0];
    camera.viewUp = [0, 0, 1];
    camera.viewAngle = 70;

    utils.assert(utils.numArrayEquals(camera.position, [0, 5, 0], 0.01));
    utils.assert(utils.numArrayEquals(camera.focalPoint, [0, 0, 0], 0.01));
    utils.assert(utils.numArrayEquals(camera.viewUp, [0, 0, 1], 0.01));
    utils.assert(Math.abs(camera.viewAngle - 70) < 0.01);

    camera.setCurrentAsDefault();

    camera.resetToBounds(0.9);

    // just call every control function
    camera.dolly(1.5).pan(1, 2, -3).zoom(0.8).roll(10).azimuth(15).yaw(-20).elevation(25).pitch(-30);

    camera.resetToDefault();
  },
};

utils.runRenderTest(settings, { data: "suzanne.ply", baseline: "TestWasmCamera.png" });

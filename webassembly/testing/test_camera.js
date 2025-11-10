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

    utils.assert(
      utils.numArrayEquals(camera.position, [0, -0.016, 7.059], 0.01),
    );
    utils.assert(
      utils.numArrayEquals(camera.focalPoint, [0, -0.016, 0.022], 0.01),
    );
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

    // Save default state
    camera.setCurrentAsDefault();

    // Test getYaw and getElevation
    console.log("\nInitial angles:");
    console.log("Yaw:", camera.getYaw());
    console.log("Elevation:", camera.getElevation());
    console.log("Azimuth:", camera.getAzimuth());

    // Test yaw rotation
    camera.yaw(45);
    console.log("Yaw:", camera.getYaw());
    console.log("Elevation:", camera.getElevation());

    utils.assert(Math.abs(camera.getYaw() - 45) < 0.01, "Yaw should be 45 after yaw(45)");
    utils.assert(Math.abs(camera.getElevation()) < 0.01, "Elevation should remain 0");

    // Test elevation rotation
    // camera.resetToDefault();
    // camera.elevation(30);
    // utils.assert(Math.abs(camera.getYaw()) < 0.01, "Yaw should remain 0");
    // utils.assert(Math.abs(camera.getElevation() - 30) < 0.01, "Elevation should be 30");

    // Reset to default state
    camera.resetToDefault();

    // Continue with existing test
    camera.resetToBounds(0.9);

    // just call every control function
    camera
      .dolly(1.5)
      .pan(1, 2, -3)
      .zoom(0.8)
      .roll(10)
      .azimuth(15)
      .yaw(-20)
      .elevation(25)
      .pitch(-30);

    camera.resetToDefault();
  },
};

utils.runRenderTest(settings, {
  data: "suzanne.ply",
  baseline: "TestWasmCamera.png",
});

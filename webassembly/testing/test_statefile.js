import utils from "./utils.js";

const settings = {
  run: async (Module) => {
    Module.Log.setVerboseLevel(Module.LogVerboseLevel.INFO, false);
    Module.Engine.autoloadPlugins();

    await utils.copyLocalFileToWasmFS(
      Module,
      "/src/testing/data/cow.vtp",
      "cow.vtp",
    );

    const engine = Module.Engine.create();
    const options = engine.getOptions();
    const scene = engine.getScene();

    options.setAsString("render.background.color", "#0000ff");
    scene.add("cow.vtp");

    // String based round trip
    const dumped = engine.dump();
    const content = dumped.toString();
    dumped.delete();
    options.setAsString("render.background.color", "#ff0000");
    const stateFromString = Module.EngineState.fromString(content);
    engine.load(stateFromString);
    stateFromString.delete();
    utils.assert(
      options.getAsString("render.background.color") === "#0000ff",
      "option should be restored from the statefile string",
    );
    utils.assert(
      scene.getAddedFiles().length === 1,
      "scene should be restored from the statefile string",
    );

    // File based round trip
    options.setAsString("render.background.color", "#0000ff");
    const stateToFile = engine.dump();
    stateToFile.toFile("/state.json");
    stateToFile.delete();
    options.setAsString("render.background.color", "#ff0000");
    const stateFromFile = Module.EngineState.fromFile("/state.json");
    engine.load(stateFromFile);
    stateFromFile.delete();
    utils.assert(
      options.getAsString("render.background.color") === "#0000ff",
      "option should be restored from the statefile file",
    );
    utils.assert(
      scene.getAddedFiles().length === 1,
      "scene should be restored from the statefile file",
    );
  },
};

settings.canvas = document.getElementById("canvas");
utils.runBasicTest(settings);

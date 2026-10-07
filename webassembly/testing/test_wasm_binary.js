import utils from "./utils.js";

wasm = fs.readFileSync("../../dist/f3d.wasm", "binary");

const settings = {
  wasmBinary: wasm,
  run: (Module) => {
    Module.Log.setVerboseLevel(Module.LogVerboseLevel.INFO, false);
    Module.Engine.autoloadPlugins();

    utils.assert(Module.Engine, "Could not load engine");
  },
};

utils.runBasicTest(settings);

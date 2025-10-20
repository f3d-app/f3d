import utils from "./utils.js";

const settings = {
  run: (Module) => {
    Module.Log.setVerboseLevel(Module.LogVerboseLevel.INFO, false);
    Module.Engine.autoloadPlugins();

    const readerOptions = Module.Engine.getAllReaderOptionNames();
    utils.assert(
      readerOptions.includes("QuakeMDL.skin_index"),
      "Cannot find reader option",
    );

    const libInfo = Module.Engine.getLibInfo();
    utils.assert(libInfo.compiler.includes("Clang"), "Cannot find compiler");
    utils.assert(libInfo.license.includes("BSD"), "Cannot find license");
    utils.assert(libInfo.modules.length > 0, "Cannot find modules");
    utils.assert(libInfo.copyrights.length > 0, "Cannot find copyrights");

    const readersInfo = Module.Engine.getReadersInfo();

    utils.assert(readersInfo.length > 0, "Cannot find readers");

    readersInfo.forEach((reader) => {
      utils.assert(
        reader.extensions.length > 0,
        "Cannot find reader extensions for " + reader.name,
      );
      utils.assert(
        reader.mimeTypes.length > 0,
        "Cannot find reader mime types for " + reader.name,
      );
      utils.assert(
        reader.hasSceneReader || reader.hasGeometryReader,
        "Cannot find reader scene or geometry support for " + reader.name,
      );
    });

    utils.assert(
      readersInfo.map((reader) => reader.name).includes("GLTF"),
      "Cannot find GLTF reader",
    );
  },
};

utils.runBasicTest(settings);

import utils from "./utils.js";

const settings = {
  run: (Module) => {
    Module.Log.setVerboseLevel(Module.LogVerboseLevel.INFO, false);

    const options = new Module.Options();

    // set a bunch of options of each type
    options.setAsString("render.background.color", "#000000");
    options.toggle("model.scivis.enable");
    options.setAsString("model.scivis.array_name", "Colors");
    options.setAsString("model.scivis.component", "-2");
    options.setAsString("model.scivis.range", "0.7,1.4"); // not used for the baseline, only for coverage

    // check that the getter works
    utils.assert(
      options.get("model.scivis.enable") === true,
      "options getter for boolean failed",
    );
    utils.assert(
      options.get("model.scivis.component") === -2,
      "options getter for int failed",
    );
    utils.assert(
      options.get("model.point_sprites.size") === 10.0,
      "options getter for double failed",
    );
    utils.assert(
      options.get("model.point_sprites.type") === "sphere",
      "options getter for string failed",
    );

    // comparing JS arrays
    utils.assert(
      JSON.stringify(options.get("model.scivis.range")) ===
        JSON.stringify([0.7, 1.4]),
      "options getter for vec<double> failed",
    );

    utils.assert(
      JSON.stringify(options.get("scene.animation.indices")) ===
        JSON.stringify([0]),
      "options getter for vec<int> failed",
    );

    // try not valued option
    utils.assert(
      options.get("model.color.opacity") === undefined,
      "options getter for undefined",
    );

    // try not existing option
    utils.assert(
      options.get("foo") === undefined,
      "options getter for not existing",
    );

    const options2 = new Module.Options();
    options2.toggle("model.scivis.enable");

    utils.assert(
      options.isSame(options2, "model.scivis.enable"),
      "options isSame",
    );
    utils.assert(
      !options.isSame(options2, "render.background.color"),
      "options !isSame",
    );
    options.copy(options2, "render.background.color");
    utils.assert(
      options.isSame(options2, "render.background.color"),
      "options isSame after copy",
    );

    utils.assert(
      options.hasValue("model.scivis.array_name"),
      "options hasValue",
    );
    utils.assert(!options.hasValue("model.color.opacity"), "options !hasValue");

    utils.assert(
      Module.Options.getAllNames().includes("ui.drop_zone.enable"),
      "options getAllNames",
    );
    utils.assert(
      options.getNames().includes("ui.drop_zone.enable"),
      "options getNames",
    );
    utils.assert(
      !options.getNames().includes("model.color.rgb"),
      "options getNames not set",
    );

    const closestOptions = options.getClosestOption("modele.colr.rgb");
    utils.assert(
      closestOptions[0] === "model.color.rgb",
      "closest option name",
    );
    utils.assert(closestOptions[1] === 2, "closest option distance");

    utils.assert(options.isOptional("model.color.rgb"), "options isOptional");
    utils.assert(
      !options.isOptional("model.point_sprites.size"),
      "options !isOptional",
    );

    options.removeValue("model.scivis.array_name");
    utils.assert(
      !options.hasValue("model.scivis.array_name"),
      "options !hasValue after removal",
    );

    options.reset("model.scivis.enable");
    utils.assert(
      options.get("model.scivis.enable") === false,
      "options getter after reset",
    );
  },
};

utils.runBasicTest(settings);

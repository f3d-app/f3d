import f3d from "/src/dist/f3d.js";

const utils = {
  assert: (condition, description) => {
    if (!condition) {
      console.error("F3D_ERROR: " + description);
    }
  },

  numArrayEquals: (arr1, arr2, epsilon = Number.EPSILON) => {
    if (arr1.length !== arr2.length) {
      return false;
    }
    for (let i = 0; i < arr1.length; i++) {
      if (Math.abs(arr1[i] - arr2[i]) > epsilon) {
        return false;
      }
    }
    return true;
  },

  printImageBase64: (Module, img) => {
    img.save("/img.png", Module.ImageSaveFormat.PNG);

    const uint8ToBase64 = (uint8Array) => {
      let binary = "";
      for (let i = 0; i < uint8Array.length; i++) {
        binary += String.fromCharCode(uint8Array[i]);
      }
      return btoa(binary);
    };

    const imgData = Module.FS.readFile("/img.png", {
      encoding: "binary",
    });
    const base64 = uint8ToBase64(imgData);

    console.log(
      "New baseline png file:\n\n\ndata:image/png;base64," + base64 + "\n\n\n",
    );
  },

  copyLocalFileToWasmFS: async (Module, localPath, wasmPath) => {
    const data = await fetch(localPath).then((b) => b.arrayBuffer());
    Module.FS.writeFile(wasmPath, new Uint8Array(data));
  },

  runBasicTest: (settings) => {
    f3d(settings)
      .then(async (Module) => {
        await Module.run(Module);

        window.close();
      })
      .catch((error) => {
        console.error('F3D_ERROR: exception thrown "' + error + '"');
        window.close();
      });
  },

  runRenderTest: (settings, args) => {
    settings.canvas = document.getElementById("canvas");

    f3d(settings)
      .then(async (Module) => {
        // write in the wasm filesystem
        await utils.copyLocalFileToWasmFS(
          Module,
          "/src/testing/data/" + args.data,
          args.data,
        );

        await utils.copyLocalFileToWasmFS(
          Module,
          "/src/testing/baselines/" + args.baseline,
          "baseline.png",
        );

        // automatically load all supported file format readers
        Module.Engine.autoloadPlugins();

        Module.engineInstance = Module.Engine.create();

        // setup the window size based on the canvas size
        const scale = window.devicePixelRatio;
        Module.engineInstance
          .getWindow()
          .setSize(
            scale * Module.canvas.clientWidth,
            scale * Module.canvas.clientHeight,
          );

        utils.assert(
          Module.engineInstance.getWindow().width ===
            scale * Module.canvas.clientWidth,
          "Failed to get width",
        );
        utils.assert(
          Module.engineInstance.getWindow().height ===
            scale * Module.canvas.clientHeight,
          "Failed to get height",
        );

        const scene = Module.engineInstance.getScene();

        utils.assert(
          scene.supports(args.data),
          args.data + " is not supported",
        );

        Module.runBefore?.(Module);

        scene.add(args.data);

        Module.runAfter?.(Module);

        // do a first render and start the interactor
        Module.engineInstance.getWindow().render();
        Module.engineInstance.getInteractor().start();

        // compare images
        const result = Module.engineInstance.getWindow().renderToImage(true);
        const baseline = new Module.Image("/baseline.png");
        const ssim = result.compare(baseline);

        if (ssim <= 0.05) {
          console.log("Passed with SSIM = " + ssim);
        } else {
          console.log("F3D_ERROR: Comparison failed with SSIM " + ssim);

          utils.printImageBase64(Module, result);
        }

        window.close();
      })
      .catch((error) => {
        console.error('F3D_ERROR: exception thrown "' + error + '"');
        window.close();
      });
  },
};

export default utils;

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

  runBasicTest: (settings) => {
    f3d(settings)
      .then(async (Module) => {
        Module.run(Module);

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
        const modelFile = await fetch("/src/testing/data/" + args.data).then(
          (b) => b.arrayBuffer(),
        );
        Module.FS.writeFile(args.data, new Uint8Array(modelFile));

        const baselineFile = await fetch(
          "/src/testing/baselines/" + args.baseline,
        ).then((b) => b.arrayBuffer());
        Module.FS.writeFile("baseline.png", new Uint8Array(baselineFile));

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

          result.save("/result.png", Module.ImageSaveFormat.PNG);

          const uint8ToBase64 = (uint8Array) => {
            let binary = "";
            for (let i = 0; i < uint8Array.length; i++) {
              binary += String.fromCharCode(uint8Array[i]);
            }
            return btoa(binary);
          };

          const resultData = Module.FS.readFile("/result.png", {
            encoding: "binary",
          });
          const base64 = uint8ToBase64(resultData);

          console.log(
            "New baseline png file:\n\n\ndata:image/png;base64," +
              base64 +
              "\n\n\n",
          );
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

import f3d from "../../dist/f3d.js";

const utils = {
  runTest: (settings) => {
    settings.canvas = document.getElementById("canvas");

    f3d(settings)
      .then(async (Module) => {
        // write in the wasm filesystem
        const modelFile = await fetch(
          "../../../testing/data/" + Module.dataPath,
        ).then((b) => b.arrayBuffer());
        Module.FS.writeFile(Module.dataPath, new Uint8Array(modelFile));

        const baselineFile = await fetch(
          "../../../testing/baselines/" + Module.baselinePath,
        ).then((b) => b.arrayBuffer());
        Module.FS.writeFile("baseline.png", new Uint8Array(baselineFile));

        // automatically load all supported file format readers
        Module.Engine.autoloadPlugins();

        Module.engineInstance = Module.Engine.create();

        Module.setupOptions(Module.engineInstance.getOptions());

        // setup the window size based on the canvas size
        const scale = window.devicePixelRatio;
        Module.engineInstance
          .getWindow()
          .setSize(
            scale * Module.canvas.clientWidth,
            scale * Module.canvas.clientHeight,
          );

        const scene = Module.engineInstance.getScene();
        scene.add(Module.dataPath);

        // do a first render and start the interactor
        Module.engineInstance.getWindow().render();
        Module.engineInstance.getInteractor().start();

        // compare images
        const result = Module.engineInstance.getWindow().renderToImage(true);
        const baseline = Module.Image.create("/baseline.png");
        const ssim = result.compare(baseline);

        if (ssim <= 0.05) {
          console.log("Passed with SSIM = " + ssim);
        } else {
          console.error("F3D_ERROR: Comparison failed with SSIM " + ssim);

          result.save("/result.png");

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

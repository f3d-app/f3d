import f3d from '../../dist/f3d.js';

const settings =
{
    canvas: document.getElementById("canvas"),
    setupOptions: (options) => {
        // background must be set to black for proper blending with transparent canvas
        options.set_color("render.background.color", 0, 0, 0);

        // setup coloring
        options.toggle("model.scivis.enable");
        options.set_string("model.scivis.array_name", "Colors");
        options.set_integer("model.scivis.component", -2);
        options.toggle("model.scivis.cells");

        // make it look nice
        options.toggle("render.effect.antialiasing.enable");
        options.toggle("render.effect.tone_mapping");
        options.toggle("render.effect.ambient_occlusion");
        options.toggle("render.hdri.ambient");

        // display widgets
        options.toggle("ui.axis");
        options.toggle("render.grid.enable");

        // default to +Z
        options.set_as_string("scene.up_direction", "+Z");
    },
    dataPath: "f3d.vtp",
    baselinePath: "TestWasmOptions.png"
};

f3d(settings)
    .then(async (Module) => {

        // write in the wasm filesystem
        const modelFile = await fetch('../../../testing/data/' + Module.dataPath).then(b => b.arrayBuffer());
        Module.FS.writeFile(Module.dataPath, new Uint8Array(modelFile));

        const baselineFile = await fetch('../../../testing/baselines/' + Module.baselinePath).then(b => b.arrayBuffer());
        Module.FS.writeFile("baseline.png", new Uint8Array(baselineFile));

        // automatically load all supported file format readers
        Module.Engine.autoloadPlugins();

        Module.engineInstance = Module.Engine.create();

        Module.setupOptions(Module.engineInstance.getOptions());

        // setup the window size based on the canvas size
        const scale = window.devicePixelRatio;
        Module.engineInstance
            .getWindow()
            .setSize(scale * Module.canvas.clientWidth, scale * Module.canvas.clientHeight);

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
            window.close();
        }
        else {
            console.error("F3D_ERROR: Comparison failed with SSIM " + ssim);

            result.save("/result.png");

            const uint8ToBase64 = (uint8Array) => {
                let binary = '';
                for (let i = 0; i < uint8Array.length; i++) {
                    binary += String.fromCharCode(uint8Array[i]);
                }
                return btoa(binary);
            };

            const resultData = Module.FS.readFile("/result.png", { encoding: 'binary' });
            const base64 = uint8ToBase64(resultData);

            const canvas = document.getElementById('canvas');
            const img = canvas.toDataURL('image/png');
            document.write('<img src="data:image/png;base64,' + base64 + '"/>');

            setTimeout(() => {
                window.close();
            }, 5000);
        }
    })
    .catch((error) => {
        console.error('F3D_ERROR: exception thrown "' + error + '"');
        window.close();
    });
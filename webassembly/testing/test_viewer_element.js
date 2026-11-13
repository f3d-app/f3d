import utils from "./utils.js";

const settings = {
  runReady: (viewer) => {
    viewer.options = {
      "render.grid.enable": true,
      "model.color.rgb": "#ff0000",
    };
  },
};

await utils.runElementRenderTest(settings, {
  baseline: "TestWasmViewerElement.png",
});

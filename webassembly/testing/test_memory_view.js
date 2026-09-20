import utils from "./utils.js";

const settings = {
  runBefore: (Module) => {
    Module.engineInstance.getScene().addMeshView({
      name: "mesh_view",
      pointCount: 4,
      points: new Float32Array([0, 0, 0, 1, 0, 0, 1, 1, 0, 0, 1, 0]),
      normals: new Float32Array([0, 0, 1, 0, 0, 1, 0, 0, 1, 0, 0, 1]),
      textureCoordinates: new Float32Array([0, 0, 1, 0, 0, 1, 1, 1]),
      polygonOffsets: new Uint32Array([0, 4]),
      polygonIndices: new Uint32Array([0, 1, 2, 3]),
    });
  },
};

utils.runRenderTest(settings, {
  baseline: "TestWasmMeshView.png",
});

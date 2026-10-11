import f3d from "f3d";

for (const id of ["viewer1", "viewer2"]) {
  const viewer = document.getElementById(id);

  viewer.addEventListener("ready", () => {
    viewer.options = {
      "render.effect.antialiasing.mode": "fxaa",
      "render.effect.tone_mapping": true,
      "render.effect.ambient_occlusion": true,
      "render.hdri.ambient": true,
    };
  });
}

// Initializing the module, must be done after adding listeners
await f3d();

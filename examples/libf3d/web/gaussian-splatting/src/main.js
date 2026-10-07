import f3d from "f3d";

const viewer = document.getElementById("viewer");
viewer.addEventListener("ready", () => {
  viewer.options = {
    "render.hdri.ambient": true,
    "model.coloring": "direct",
    "model.point_sprites.type": "gaussian",
    "model.point_sprites.size": 1,
    "model.point_sprites.absolute_size": true,
    "render.effect.blending.mode": "stochastic",
    "render.effect.antialiasing.mode": "taa",
  };
});

await f3d();

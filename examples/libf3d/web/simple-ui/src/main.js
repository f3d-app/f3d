import "bulma/css/bulma.min.css";
import "bulma-switch/dist/css/bulma-switch.min.css";
import f3d from "f3d";

const viewer = document.getElementById("viewer");
viewer.addEventListener("ready", () => {
  // setup options
  viewer.options = {
    "render.effect.antialiasing.mode": "fxaa",
    "render.effect.tone_mapping": true,
    "render.effect.ambient_occlusion": true,
    "render.hdri.ambient": true,
    "model.coloring": "direct",
    "model.scivis.array_name": "Colors",
    "ui.axis": true,
    "render.grid.enable": true,
    "scene.up_direction": "+Z",
  };

  const openFile = (name, stream) => {
    document.getElementById("file-name").innerHTML = name;

    try {
      viewer.load(stream);
    } catch (e) {
      document.getElementById("file-name").innerHTML =
        '<strong class="has-text-danger">Unsupported file</strong>';
    }
  };

  // setup file open event
  const fileSelector = document.querySelector("#file-selector");
  fileSelector.addEventListener("change", (evt) => {
    for (const file of evt.target.files) {
      const reader = new FileReader();
      reader.addEventListener("loadend", (e) => {
        openFile(file.name, new Uint8Array(reader.result));
      });
      reader.readAsArrayBuffer(file);
    }
  });

  // Storing DOM element ids to f3d option mappings since also useful for url-param parsing
  const idOptionMappings = [
    ["grid", "render.grid.enable"],
    ["axis", "ui.axis"],
    ["tone", "render.effect.tone_mapping"],
    ["ssao", "render.effect.ambient_occlusion"],
    ["ambient", "render.hdri.ambient"],
  ];

  // toggle callback
  const mapToggleIdToOption = (id, option) => {
    document.querySelector("#" + id).addEventListener("change", (evt) => {
      viewer.engine.getOptions().toggle(option);
      viewer.engine.getWindow().render();
    });
  };

  // This assumes all toggles are 'on' before mapping their state to options
  // Ok after f3d(settings) where settings = {..., setupOptions} which toggles some options
  for (let [id, option] of idOptionMappings) {
    mapToggleIdToOption(id, option);
  }

  const switchDark = () => {
    document.documentElement.classList.add("theme-dark");
    document.documentElement.classList.remove("theme-light");
    viewer.engine
      .getOptions()
      .setAsString("render.grid.color", "0.25, 0.27, 0.33");
    viewer.engine.getWindow().render();
  };

  const switchLight = () => {
    document.documentElement.classList.add("theme-light");
    document.documentElement.classList.remove("theme-dark");
    viewer.engine
      .getOptions()
      .setAsString("render.grid.color", "0.67, 0.69, 0.75");
    viewer.engine.getWindow().render();
  };

  // theme switch
  document.querySelector("#dark").addEventListener("change", (evt) => {
    if (evt.target.checked) switchDark();
    else switchLight();
  });

  switchDark();

  // up callback
  document.querySelector("#z-up").addEventListener("click", (evt) => {
    viewer.engine.getOptions().setAsString("scene.up_direction", "+Z");
    document.getElementById("z-up").classList.add("is-active");
    document.getElementById("y-up").classList.remove("is-active");
  });

  document.querySelector("#y-up").addEventListener("click", (evt) => {
    viewer.engine.getOptions().setAsString("scene.up_direction", "+Y");
    document.getElementById("y-up").classList.add("is-active");
    document.getElementById("z-up").classList.remove("is-active");
  });
});

await f3d();

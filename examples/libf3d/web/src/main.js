import "bulma/css/bulma.min.css";
import "bulma-switch/dist/css/bulma-switch.min.css";
import f3d from "f3d";

const settings = {
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
};

f3d(settings)
  .then(async (Module) => {
    // write in the filesystem
    const defaultFile = await fetch("f3d.vtp").then((b) => b.arrayBuffer());
    Module.FS.writeFile("f3d.vtp", new Uint8Array(defaultFile));

    // automatically load all supported file format readers
    Module.Engine.autoloadPlugins();

    Module.engineInstance = Module.Engine.create();

    const openFile = (name) => {
      document.getElementById("file-name").innerHTML = name;
      const filePath = "/" + name;
      const scene = Module.engineInstance.getScene();
      if (scene.supports(filePath)) {
        scene.clear();
        scene.add(filePath);
      } else {
        console.error("File " + filePath + " cannot be opened");
      }
      Module.engineInstance.getWindow().resetCamera();
      Module.engineInstance.getWindow().render();
    };

    // setup file open event
    const progressEl = document.querySelector("#progressEl");
    const fileSelector = document.querySelector("#file-selector");
    fileSelector.addEventListener("change", (evt) => {
      for (const file of evt.target.files) {
        const reader = new FileReader();
        reader.addEventListener("loadend", (e) => {
          Module.FS.writeFile(file.name, new Uint8Array(reader.result));
          openFile(file.name);
        });
        reader.readAsArrayBuffer(file);
      }
    });

    Module.setupOptions(Module.engineInstance.getOptions());

    // Storing DOM element ids to f3d option mappings since also useful for url-param parsing
    const idOptionMappings = [
      ["grid", "render.grid.enable"],
      ["axis", "ui.axis"],
      ["fxaa", "render.effect.antialiasing.enable"],
      ["tone", "render.effect.tone_mapping"],
      ["ssao", "render.effect.ambient_occlusion"],
      ["ambient", "render.hdri.ambient"],
    ];

    // toggle callback
    const mapToggleIdToOption = (id, option) => {
      document.querySelector("#" + id).addEventListener("change", (evt) => {
        Module.engineInstance.getOptions().toggle(option);
        Module.engineInstance.getWindow().render();
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
      Module.engineInstance
        .getOptions()
        .set_color("render.grid.color", 0.25, 0.27, 0.33);
      Module.engineInstance.getWindow().render();
    };

    const switchLight = () => {
      document.documentElement.classList.add("theme-light");
      document.documentElement.classList.remove("theme-dark");
      Module.engineInstance
        .getOptions()
        .set_color("render.grid.color", 0.67, 0.69, 0.75);
      Module.engineInstance.getWindow().render();
    };

    // theme switch
    document.querySelector("#dark").addEventListener("change", (evt) => {
      if (evt.target.checked) switchDark();
      else switchLight();
    });

    switchDark();

    // up callback
    document.querySelector("#z-up").addEventListener("click", (evt) => {
      Module.engineInstance
        .getOptions()
        .set_as_string("scene.up_direction", "+Z");
      document.getElementById("z-up").classList.add("is-active");
      document.getElementById("y-up").classList.remove("is-active");
      openFile(document.getElementById("file-name").innerHTML);
    });

    document.querySelector("#y-up").addEventListener("click", (evt) => {
      Module.engineInstance
        .getOptions()
        .set_as_string("scene.up_direction", "+Y");
      document.getElementById("y-up").classList.add("is-active");
      document.getElementById("z-up").classList.remove("is-active");
      openFile(document.getElementById("file-name").innerHTML);
    });

    // setup the window size based on the canvas size
    const main = document.getElementById("main");
    const scale = window.devicePixelRatio;
    Module.engineInstance
      .getWindow()
      .setSize(scale * main.clientWidth, scale * main.clientHeight);

    // do a first render and start the interactor
    Module.engineInstance.getWindow().render();
    Module.engineInstance.getInteractor().start();
  })
  .catch((error) => console.error("Internal exception: " + error));

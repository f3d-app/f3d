import * as d3 from "d3";
import f3d from "f3d";

function clamp(v) {
  return Math.max(0, Math.min(1, v));
}

// Setup the transfer-function editor SVG
const tfSvg = d3.select("#tfEditor");
const tfWidth = 256,
  tfHeight = 96;

const tfX = d3.scaleLinear().domain([0, 1]).range([0, tfWidth]);
const tfY = d3.scaleLinear().domain([0, 1]).range([tfHeight, 0]);
const tfLine = d3
  .line()
  .x((d) => tfX(d.x))
  .y((d) => tfY(d.y));

// colormap background
const tfDefs = tfSvg.append("defs");
const tfGrad = tfDefs
  .append("linearGradient")
  .attr("id", "tf-cm-grad")
  .attr("x1", "0%")
  .attr("x2", "100%")
  .attr("y1", "0%")
  .attr("y2", "0%");
const tfBg = tfSvg
  .append("rect")
  .attr("x", 0)
  .attr("y", 0)
  .attr("width", tfWidth)
  .attr("height", tfHeight)
  .attr("fill", "url(#tf-cm-grad)");

const tfPath = tfSvg
  .append("path")
  .attr("fill", "none")
  .attr("stroke", "#fff")
  .attr("stroke-width", 2);
const tfNodes = tfSvg.append("g");

// colormap
const colormapInterpolators = {
  viridis: d3.interpolateViridis,
  inferno: d3.interpolateInferno,
  magma: d3.interpolateMagma,
  plasma: d3.interpolatePlasma,
  cividis: d3.interpolateCividis,
  turbo: d3.interpolateTurbo,
  warm: d3.interpolateWarm,
  cool: d3.interpolateCool,
  spectral: d3.interpolateSpectral,
  rainbow: d3.interpolateRainbow,
};

// Generate F3D colormap string format: "val,#rrggbb,val,#rrggbb,..."
function buildColormapString(name, nSamples = 32) {
  const interp = colormapInterpolators[name];
  if (!interp) return null;
  return Array.from({ length: nSamples }, (_, i) => {
    const t = i / (nSamples - 1);
    return `${t},${d3.color(interp(t)).formatHex()}`;
  }).join(",");
}

// Update colormap background gradient in the TF editor
function updateTFBackground(name, nStops = 16) {
  const interp = colormapInterpolators[name];
  if (!interp) return;
  tfGrad
    .selectAll("stop")
    .data(d3.range(nStops))
    .join("stop")
    .attr("offset", (i) => `${(i / (nStops - 1)) * 100}%`)
    .attr("stop-color", (i) => interp(i / (nStops - 1)));
}

// Initial state of the transfer function editor
let onTFChange = () => {};
let currentColormap = "viridis";
let tfPoints = [
  { x: 0, y: 0 },
  { x: 1, y: 1 },
];

function renderTF() {
  tfPoints.sort((a, b) => a.x - b.x);
  tfPath.attr("d", tfLine(tfPoints));

  tfNodes
    .selectAll("circle")
    .data(tfPoints)
    .join(
      (enter) =>
        enter
          .append("circle")
          .attr("r", 5)
          .attr("fill", "#fff")
          .attr("stroke", "#333")
          .attr("cx", (d) => tfX(d.x))
          .attr("cy", (d) => tfY(d.y))
          .on("dblclick", (event, d) => {
            // remove point
            event.stopPropagation();
            if (tfPoints.length <= 2) return;
            tfPoints = tfPoints.filter((p) => p !== d);
            renderTF();
          })
          .call(
            d3
              .drag()
              .on("start", (e) => e.sourceEvent.stopPropagation())
              .on("drag", (e, d) => {
                const [mx, my] = d3.pointer(e, tfSvg.node());
                d.x = clamp(tfX.invert(mx));
                d.y = clamp(tfY.invert(my));
                renderTF();
              }),
          ),
      (update) =>
        update.attr("cx", (d) => tfX(d.x)).attr("cy", (d) => tfY(d.y)),
      (exit) => exit.remove(),
    );

  onTFChange();
  updateTFBackground(currentColormap);
}

tfSvg.on("dblclick", (e) => {
  // add point
  const [mx, my] = d3.pointer(e, tfSvg.node());
  tfPoints.push({ x: clamp(tfX.invert(mx)), y: clamp(tfY.invert(my)) });
  renderTF();
});

renderTF();

let onColormapChange = () => {};
document
  .getElementById("colormap")
  .addEventListener("change", (e) => onColormapChange(e.target.value));

f3d({})
  .then(async (Module) => {
    Module.Log.setVerboseLevel(Module.LogVerboseLevel.QUIET, false);
    Module.Log.forward((level, message) => {
      if (level === Module.LogVerboseLevel.ERROR) console.error(message);
      else if (level === Module.LogVerboseLevel.WARN) console.warn(message);
      else if (level === Module.LogVerboseLevel.INFO) console.info(message);
    });

    // automatically load all supported file format readers
    Module.Engine.autoloadPlugins();

    const engine = Module.Engine.create();

    // background must be set to black for proper blending with transparent canvas
    engine.getOptions().setAsString("render.background.color", "#000000");
    engine.getOptions().setAsString("scene.up_direction", "+z");
    engine.getOptions().toggle("model.volume.enable");
    engine.getOptions().toggle("ui.scalar_bar");

    // setup the window size based on the canvas size
    const canvas = document.getElementById("canvas");
    const scale = window.devicePixelRatio;
    engine
      .getWindow()
      .setSize(scale * canvas.clientWidth, scale * canvas.clientHeight);

    // read file and display it
    const response = await fetch(`https://f3d.app/data/skull.vti`);
    const arrayBuffer = await response.arrayBuffer();
    const scene = engine.getScene();
    try {
      scene.addBuffer(new Uint8Array(arrayBuffer));
    } catch (e) {
      console.error("Unsupported file");
    }

    const options = engine.getOptions();

    onTFChange = () => {
      const sorted = tfPoints.sort((a, b) => a.x - b.x);

      // update opacity_map
      options.setAsString(
        "model.scivis.opacity_map",
        sorted.map((p) => `${p.x},${p.y}`).join(","),
      );

      // update range
      options.setAsString(
        "model.scivis.range",
        `${255 * sorted[0].x},${255 * sorted[sorted.length - 1].x}`,
      );
      engine.getWindow().render();
    };

    onColormapChange = (name) => {
      currentColormap = name;
      const cm = buildColormapString(name);
      if (cm) {
        options.setAsString("model.scivis.colormap", cm);
        updateTFBackground(name);
        engine.getWindow().render();
      }
    };

    // apply initial values
    onTFChange();
    onColormapChange(document.getElementById("colormap").value);

    // do a first render and start the interactor
    engine.getWindow().render();
    engine.getInteractor().start();
  })
  .catch((error) => console.error("Internal exception: " + error));

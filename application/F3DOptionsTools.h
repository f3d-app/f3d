#ifndef F3DOptionsTools_h
#define F3DOptionsTools_h

/**
 * @class   F3DOptionsTools
 * @brief   A namespace to handle and parse F3D Options
 */
#include <filesystem>
#include <map>
#include <string>
#include <tuple>
#include <vector>

namespace F3DOptionsTools
{
using OptionsDict = std::map<std::string, std::string>;
using OptionsEntry = std::tuple<OptionsDict, std::string, std::string, std::string>;
using OptionsEntries = std::vector<OptionsEntry>;

/**
 * The "App" CLI options and their default values as string.
 * Some F3D CLI options are not translated into libf3d options
 * but into applicative code.
 * Initialization is string based for easier processing in F3DStarter::UpdateOptions
 * and ParseCLIOptions
 */
static inline const OptionsDict DefaultAppOptions = {
  { "input", "" },
  { "output", "" },
  { "list-bindings", "false" },
  { "no-background", "false" },
  { "config", "" },
  { "no-config", "false" },
  { "no-render", "false" },
  { "rendering-backend", "auto" },
  { "max-size", "" },
  { "animation-time", "" },
  { "watch", "false" },
  { "load-plugins", "" },
  { "screenshot-filename", "{app}/{model}_{n}.png" },
  { "verbose", "info" },
  { "multi-file-mode", "single" },
  { "multi-file-regex", "" },
  { "recursive-dir-add", "false" },
  { "remove-empty-file-groups", "false" },
  { "resolution", "1000, 600" },
  { "position", "" },
  { "colormap-file", "" },
  { "camera-position", "" },
  { "camera-focal-point", "" },
  { "camera-view-up", "" },
  { "camera-view-angle", "0.0" },
  { "camera-direction", "" },
  { "camera-zoom-factor", "0.0" },
  { "camera-azimuth-angle", "0.0" },
  { "camera-elevation-angle", "0.0" },
  { "reference", "" },
  { "reference-threshold", "0.04" },
  { "interaction-test-record", "" },
  { "interaction-test-play", "" },
  { "command-script", "" },
  { "frame-rate", "30.0" },
};

/**
 * Mapping of CLI option name to their libf3d options name counterpart
 */
static inline const std::map<std::string_view, std::string_view> LibOptionsNames = {
  { "loading-progress", "ui.loader_progress" },
  { "animation-progress", "ui.animation_progress" },
  { "up", "scene.up_direction" },
  { "axis", "ui.axis" },
  { "grid", "render.grid.enable" },
  { "grid-absolute", "render.grid.absolute" },
  { "grid-unit", "render.grid.unit" },
  { "grid-subdivisions", "render.grid.subdivisions" },
  { "grid-color", "render.grid.color" },
  { "axes-grid", "render.axes_grid.enable" },
  { "edges", "render.show_edges" },
  { "armature", "render.armature.enable" },
  { "camera-index", "scene.camera.index" },
  { "interaction-trackball", "interactor.trackball" },
  { "invert-zoom", "interactor.invert_zoom" },
  { "animation-autoplay", "scene.animation.autoplay" },
  { "animation-index", "scene.animation.index" },
  { "animation-indices", "scene.animation.indices" },
  { "animation-speed-factor", "scene.animation.speed_factor" },
  { "force-reader", "scene.force_reader" },
  { "font-file", "ui.font_file" },
  { "font-scale", "ui.scale" },
  { "point-sprites", "model.point_sprites.enable" },
  { "point-sprites-type", "model.point_sprites.type" },
  { "point-sprites-size", "model.point_sprites.size" },
  { "point-size", "render.point_size" },
  { "line-width", "render.line_width" },
  { "backface-type", "render.backface_type" },
  { "color", "model.color.rgb" },
  { "opacity", "model.color.opacity" },
  { "roughness", "model.material.roughness" },
  { "metallic", "model.material.metallic" },
  { "base-ior", "model.material.base_ior" },
  { "hdri-file", "render.hdri.file" },
  { "hdri-ambient", "render.hdri.ambient" },
  { "hdri-skybox", "render.background.skybox" },
  { "texture-matcap", "model.matcap.texture" },
  { "texture-base-color", "model.color.texture" },
  { "texture-material", "model.material.texture" },
  { "texture-emissive", "model.emissive.texture" },
  { "emissive-factor", "model.emissive.factor" },
  { "texture-normal", "model.normal.texture" },
  { "normal-scale", "model.normal.scale" },
  { "background-color", "render.background.color" },
  { "fps", "ui.fps" },
  { "filename", "ui.filename" },
  { "metadata", "ui.metadata" },
  { "blur-background", "render.background.blur.enable" },
  { "blur-coc", "render.background.blur.coc" },
  { "scalar-coloring", "model.scivis.enable" },
  { "coloring-array", "model.scivis.array_name" },
  { "light-intensity", "render.light.intensity" },
  { "coloring-component", "model.scivis.component" },
  { "coloring-by-cells", "model.scivis.cells" },
  { "coloring-range", "model.scivis.range" },
  { "coloring-scalar-bar", "ui.scalar_bar" },
  { "colormap", "model.scivis.colormap" },
  { "colormap-discretization", "model.scivis.discretization" },
  { "volume", "model.volume.enable" },
  { "volume-inverse", "model.volume.inverse" },
  { "camera-orthographic", "scene.camera.orthographic" },
  { "raytracing", "render.raytracing.enable" },
  { "raytracing-samples", "render.raytracing.samples" },
  { "raytracing-denoise", "render.raytracing.denoise" },
  { "translucency-support", "render.effect.translucency_support" },
  { "ambient-occlusion", "render.effect.ambient_occlusion" },
  { "anti-aliasing", "render.effect.antialiasing.enable" },
  { "anti-aliasing-mode", "render.effect.antialiasing.mode" },
  { "tone-mapping", "render.effect.tone_mapping" },
  { "final-shader", "render.effect.final_shader" },
  { "textures-transform", "model.textures_transform" },
};

/**
 * Browse through all possible option names to find one that have the smallest distance to the
 * provided option.
 * If checkLibAndReaders is true, even check in the libf3d and reader option names from plugins
 * Return a pair containing the closest option name and the associated distance
 */
std::pair<std::string, int> GetClosestOption(
  const std::string& option, bool checkLibAndReaders = false);

/**
 * Parse CLI options from provided argc/argv and returns them as a OptionsDict.
 * Also set positionals vector in the process.
 * Do not parse option value into actual values.
 */
F3DOptionsTools::OptionsDict ParseCLIOptions(
  int argc, char** argv, std::vector<std::string>& positionals);

/**
 * Log provided key and help with nice formatting
 */
void PrintHelpPair(
  std::string_view key, std::string_view help, int keyWidth = 10, int helpWidth = 70);
};

#endif

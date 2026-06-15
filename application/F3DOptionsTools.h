#ifndef F3DOptionsTools_h
#define F3DOptionsTools_h

/**
 * @class   F3DOptionsTools
 * @brief   A namespace to handle and parse F3D Options
 */
#include <options.h>

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
  { "plugins-path", "" },
  { "screenshot-filename", "{app}/{model}_{n}.png" },
  { "verbose", "info" },
  { "multi-file-mode", "single" },
  { "multi-file-regex", "" },
  { "recursive-dir-add", "false" },
  { "remove-empty-file-groups", "false" },
  { "resolution", "1000, 600" },
  { "position", "" },
  { "colormap-file", "" },
  { "volume-opacity-file", "" },
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
  { "ambient-occlusion", "render.effect.ambient_occlusion" },
  { "animation-autoplay", "scene.animation.autoplay" },
  { "animation-index", "scene.animation.index" },
  { "animation-indices", "scene.animation.indices" },
  { "animation-progress", "ui.animation_progress" },
  { "animation-speed-factor", "scene.animation.speed_factor" },
  { "anti-aliasing", "render.effect.antialiasing.mode" },
  { "armature", "render.armature.enable" },
  { "axes-grid", "render.axes_grid.enable" },
  { "axis", "ui.axis" },
  { "backdrop-color", "ui.backdrop.color" },
  { "backdrop-opacity", "ui.backdrop.opacity" },
  { "backface-type", "render.backface_type" },
  { "background-color", "render.background.color" },
  { "base-ior", "model.material.base_ior" },
  { "blur-background", "render.background.blur.enable" },
  { "blur-coc", "render.background.blur.coc" },
  { "camera-index", "scene.camera.index" },
  { "camera-orthographic", "scene.camera.orthographic" },
  { "checkerboard", "model.checkerboard.enable" },
  { "color", "model.color.rgb" },
  { "coloring-array", "model.scivis.array_name" },
  { "coloring-by-cells", "model.scivis.cells" },
  { "coloring-component", "model.scivis.component" },
  { "coloring-range", "model.scivis.range" },
  { "coloring-scalar-bar", "ui.scalar_bar" },
  { "colormap", "model.scivis.colormap" },
  { "colormap-discretization", "model.scivis.discretization" },
  { "display-depth", "render.effect.display_depth" },
  { "dpi-aware", "ui.dpi_aware" },
  { "edges", "render.show_edges" },
  { "emissive-factor", "model.emissive.factor" },
  { "filename", "ui.filename" },
  { "final-shader", "render.effect.final_shader" },
  { "font-color", "ui.font_color" },
  { "font-file", "ui.font_file" },
  { "font-scale", "ui.scale" },
  { "force-reader", "scene.force_reader" },
  { "fps", "ui.fps" },
  { "grid", "render.grid.enable" },
  { "grid-absolute", "render.grid.absolute" },
  { "grid-color", "render.grid.color" },
  { "grid-reflection", "render.grid.reflection" },
  { "grid-subdivisions", "render.grid.subdivisions" },
  { "grid-unit", "render.grid.unit" },
  { "hdri-ambient", "render.hdri.ambient" },
  { "hdri-file", "render.hdri.file" },
  { "hdri-filename", "ui.hdri_filename" },
  { "hdri-skybox", "render.background.skybox" },
  { "interaction-style", "interactor.style" },
  { "invert-zoom", "interactor.invert_zoom" },
  { "light-intensity", "render.light.intensity" },
  { "line-width", "render.line_width" },
  { "loading-progress", "ui.loader_progress" },
  { "metadata", "ui.metadata" },
  { "metallic", "model.material.metallic" },
  { "normal-glyphs", "model.normal_glyphs.enable" },
  { "normal-glyphs-scale", "model.normal_glyphs.scale" },
  { "normal-scale", "model.normal.scale" },
  { "notifications", "ui.notifications.enable" },
  { "opacity", "model.color.opacity" },
  { "point-size", "render.point_size" },
  { "point-sprites-absolute-size", "model.point_sprites.absolute_size" },
  { "point-sprites-size", "model.point_sprites.size" },
  { "raytracing", "render.raytracing.enable" },
  { "raytracing-denoise", "render.raytracing.denoise" },
  { "raytracing-samples", "render.raytracing.samples" },
  { "roughness", "model.material.roughness" },
  { "scalar-coloring", "model.scivis.enable" },
  { "scene-hierarchy", "ui.scene_hierarchy" },
  { "texture-base-color", "model.color.texture" },
  { "texture-emissive", "model.emissive.texture" },
  { "texture-matcap", "model.matcap.texture" },
  { "texture-material", "model.material.texture" },
  { "texture-normal", "model.normal.texture" },
  { "textures-transform", "model.textures_transform" },
  { "tone-mapping", "render.effect.tone_mapping" },
  { "unlit", "model.unlit" },
  { "up", "scene.up_direction" },
  { "volume", "model.volume.enable" },
  { "volume-inverse", "model.volume.inverse" },
  { "volume-opacity-map", "model.scivis.opacity_map" },
  { "x-color", "ui.x_color" },
  { "y-color", "ui.y_color" },
  { "z-color", "ui.z_color" },
};

/**
 * List of CLI option names that requires custom mapping
 */
static inline const std::map<std::string_view, std::string_view> CustomMappingOptions = {
  { "interaction-trackball", "false" },
  { "translucency-support", "false" },
  { "blending", "none" },
  { "point-sprites", "none" },
  { "point-sprites-type", "" },
};

/**
 * Convert a CLI options key/value to a vector of libf3d options key/value
 */
std::vector<std::pair<std::string, std::string>> ConvertToLibf3dOptions(
  const std::string& key, const std::string& value);

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

/**
 * Parse provided string into provided typed var.
 * Return true if successful, false otherwise.
 */
template<typename T>
bool Parse(const std::string& optionString, T& option)
{
  try
  {
    option = f3d::options::parse<T>(optionString);
    return true;
  }
  catch (const f3d::options::parsing_exception&)
  {
    return false;
  }
}

};

#endif

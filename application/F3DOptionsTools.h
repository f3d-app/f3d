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
using OptionsEntry = std::tuple<OptionsDict, std::filesystem::path, std::string>;
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
  { "no-background", "false" },
  { "config", "" },
  { "dry-run", "false" },
  { "no-render", "false" },
  { "rendering-backend", "auto" },
  { "max-size", "-1.0" },
  { "watch", "false" },
  { "load-plugins", "" },
  { "screenshot-filename", "{app}/{model}_{n}.png" },
  { "verbose", "info" },
  { "multi-file-mode", "single" },
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
  { "ref", "" },
  { "ref-threshold", "0.05" },
  { "interaction-test-record", "" },
  { "interaction-test-play", "" },
  { "command-script", "" },
};

/**
 * Mapping of CLI option name to their libf3d options name counterpart
 */
static inline const std::map<std::string_view, std::string_view> LibOptionsNames = {
  { "progress", "ui.loader_progress" },
  { "animation-progress", "ui.animation_progress" },
  { "up", "scene.up_direction" },
  { "axis", "interactor.axis" },
  { "grid", "render.grid.enable" },
  { "grid-absolute", "render.grid.absolute" },
  { "grid-unit", "render.grid.unit" },
  { "grid-subdivisions", "render.grid.subdivisions" },
  { "grid-color", "render.grid.color" },
  { "edges", "render.show_edges" },
  { "camera-index", "scene.camera.index" },
  { "trackball", "interactor.trackball" },
  { "invert-zoom", "interactor.invert_zoom" },
  { "animation-autoplay", "scene.animation.autoplay" },
  { "animation-index", "scene.animation.index" },
  { "animation-speed-factor", "scene.animation.speed_factor" },
  { "animation-time", "scene.animation.time" },
  { "animation-frame-rate", "scene.animation.frame_rate" },
  { "font-file", "ui.font_file" },
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
  { "bg-color", "render.background.color" },
  { "fps", "ui.fps" },
  { "filename", "ui.filename" },
  { "metadata", "ui.metadata" },
  { "blur-background", "render.background.blur" },
  { "blur-coc", "render.background.blur_coc" },
  { "scalar-coloring", "model.scivis.enable" },
  { "coloring-array", "model.scivis.array_name" },
  { "light-intensity", "render.light.intensity" },
  { "comp", "model.scivis.component" },
  { "cells", "model.scivis.cells" },
  { "range", "model.scivis.range" },
  { "bar", "ui.scalar_bar" },
  { "colormap", "model.scivis.colormap" },
  { "volume", "model.volume.enable" },
  { "inverse", "model.volume.inverse" },
  { "camera-orthographic", "scene.camera.orthographic" },
  { "raytracing", "render.raytracing.enable" },
  { "samples", "render.raytracing.samples" },
  { "denoise", "render.raytracing.denoise" },
  { "translucency-support", "render.effect.translucency_support" },
  { "ambient-occlusion", "render.effect.ambient_occlusion" },
  { "anti-aliasing", "render.effect.anti_aliasing" },
  { "tone-mapping", "render.effect.tone_mapping" },
  { "final-shader", "render.effect.final_shader" },
};

/**
 * Browse through all possible option names to find one that have the smallest distance to the
 * provided option.
 * If checkLib is true, even check in the libf3d option names
 * Return a pair containing the closest option name and the associated distance
 */
std::pair<std::string, int> GetClosestOption(const std::string& option, bool checkLib = false);

/**
 * Parse CLI options from provided argc/argv and returns them as a OptionsDict.
 * Also set positionals vector in the process.
 * Do not parse option value into actual values.
 */
F3DOptionsTools::OptionsDict ParseCLIOptions(
  int argc, char** argv, std::vector<std::string>& positionals);
};

#endif

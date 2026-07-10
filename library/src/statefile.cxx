#include "statefile.h"

#include "camera.h"
#include "engine.h"
#include "log.h"
#include "options.h"
#include "scene.h"
#include "window.h"

#include <nlohmann/json.hpp>

#include <algorithm>
#include <filesystem>
#include <iterator>

namespace fs = std::filesystem;

namespace f3d::detail
{
//----------------------------------------------------------------------------
std::string captureStateContent(scene& scene, window& window, options& options)
{
  nlohmann::ordered_json root;

  // Store the added files as absolute paths, the canonical form of a state (see RelativizeFiles).
  nlohmann::ordered_json files = nlohmann::ordered_json::array();
  std::ranges::transform(scene.getAddedFiles(), std::back_inserter(files),
    [](const fs::path& file) { return fs::absolute(file).generic_string(); });
  if (files.empty())
  {
    log::info("No files to save in statefile");
  }
  root["files"] = files;

  // Camera and window geometry, only if a window is available
  if (window.getType() != f3d::window::Type::NONE)
  {
    const f3d::camera_state_t state = window.getCamera().getState();
    nlohmann::ordered_json camera;
    camera["position"] = { state.position[0], state.position[1], state.position[2] };
    camera["focal_point"] = { state.focalPoint[0], state.focalPoint[1], state.focalPoint[2] };
    camera["view_up"] = { state.viewUp[0], state.viewUp[1], state.viewUp[2] };
    camera["view_angle"] = state.viewAngle;
    root["camera"] = camera;

    nlohmann::ordered_json windowJson;
    windowJson["width"] = window.getWidth();
    windowJson["height"] = window.getHeight();
    // TODO: also save/restore the window position (getPositionX/getPositionY, setPosition) once VTK
    // is fixed. https://gitlab.kitware.com/vtk/vtk/-/work_items/20112
    root["window"] = windowJson;
  }

  nlohmann::ordered_json optionsJson = nlohmann::ordered_json::object();
  for (const std::string& name : options.getNames())
  {
    optionsJson[name] = options.getAsString(name);
  }
  root["options"] = optionsJson;

  return root.dump(2);
}

//----------------------------------------------------------------------------
void restoreStateContent(
  scene& scene, window& window, options& options, const std::string& content)
{
  nlohmann::ordered_json root;
  try
  {
    root = nlohmann::ordered_json::parse(content);
  }
  catch (const nlohmann::json::exception& ex)
  {
    throw engine::statefile_exception(std::string("Could not parse state content: ") + ex.what());
  }

  // Clear the scene first so that the statefile fully replaces the current state
  scene.clear();

  if (root.contains("options"))
  {
    for (const auto& [name, value] : root.at("options").items())
    {
      try
      {
        options.setAsString(name, value.get<std::string>());
      }
      catch (const f3d::options::inexistent_exception&)
      {
        log::warn("Statefile option \"", name, "\" does not exist, skipping it");
      }
      catch (const f3d::options::parsing_exception&)
      {
        log::warn("Statefile option \"", name, "\" could not be parsed from value \"",
          value.get<std::string>(), "\", skipping it");
      }
    }
  }
  else
  {
    log::warn("No options found in statefile");
  }

  // Add the saved files as is: a state always holds absolute paths.
  if (root.contains("files"))
  {
    std::vector<fs::path> files;
    std::ranges::transform(root.at("files"), std::back_inserter(files),
      [](const nlohmann::ordered_json& file) { return fs::path(file.get<std::string>()); });
    if (!files.empty())
    {
      // Let any scene::load_failure_exception propagate to the caller: a statefile that cannot
      // reload its files is a failure to restore the state, not something to silently ignore.
      scene.add(files);
    }
  }

  const bool hasWindow = window.getType() != f3d::window::Type::NONE;

  if (root.contains("camera"))
  {
    if (hasWindow)
    {
      const nlohmann::ordered_json& camera = root.at("camera");
      const auto& pos = camera.at("position");
      const auto& foc = camera.at("focal_point");
      const auto& up = camera.at("view_up");
      f3d::camera_state_t state;
      state.position = { pos[0].get<double>(), pos[1].get<double>(), pos[2].get<double>() };
      state.focalPoint = { foc[0].get<double>(), foc[1].get<double>(), foc[2].get<double>() };
      state.viewUp = { up[0].get<double>(), up[1].get<double>(), up[2].get<double>() };
      state.viewAngle = camera.at("view_angle").get<double>();
      window.getCamera().setState(state);
    }
    else
    {
      log::info("No window available, skipping camera state from statefile");
    }
  }

  if (root.contains("window"))
  {
    if (hasWindow)
    {
      const nlohmann::ordered_json& windowJson = root.at("window");
      window.setSize(windowJson.at("width").get<int>(), windowJson.at("height").get<int>());
    }
    else
    {
      log::info("No window available, skipping window size from statefile");
    }
  }
}
}

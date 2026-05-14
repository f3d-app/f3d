#include "F3DPluginsTools.h"

#include "F3DConfig.h"
#include "F3DSystemTools.h"

#include <engine.h>
#include <log.h>

namespace fs = std::filesystem;

namespace
{
//----------------------------------------------------------------------------
std::vector<fs::path> GetPluginSearchPaths(const std::string& pluginsPath)
{
  std::vector<fs::path> searchPaths;

  // Recover user provided plugins path first
  if (!pluginsPath.empty())
  {
    searchPaths.emplace_back(pluginsPath);
  }

#if F3D_MACOS_BUNDLE
  return searchPaths;
#else
  // Add a executable related path
  auto tmpPath = F3DSystemTools::GetApplicationPath();
  tmpPath = tmpPath.parent_path().parent_path();
  tmpPath /= F3D::PluginsInstallDir;
  searchPaths.emplace_back(tmpPath);
  return searchPaths;
#endif
}
};

//----------------------------------------------------------------------------
void F3DPluginsTools::LoadPlugins(
  const std::vector<std::string>& plugins, const std::string& pluginsPath)
{
  try
  {
    const std::vector pluginsPaths = ::GetPluginSearchPaths(pluginsPath);

    f3d::engine::autoloadPlugins();

    for (const std::string& plugin : plugins)
    {
      if (!plugin.empty())
      {
        f3d::engine::loadPlugin(plugin, pluginsPaths);
      }
    }
  }
  catch (const f3d::engine::plugin_exception& e)
  {
    f3d::log::warn("Plugin failed to load: ", e.what());
  }
}

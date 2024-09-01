#include "F3DPluginsTools.h"

#include "F3DConfig.h"
#include "F3DSystemTools.h"

#include <engine.h>
#include <log.h>

namespace
{
//----------------------------------------------------------------------------
std::vector<std::string> GetPluginSearchPaths()
{
  // Recover F3D_PLUGINS_PATH first
  auto searchPaths = F3DSystemTools::GetVectorEnvironnementVariable("F3D_PLUGINS_PATH");
#if F3D_MACOS_BUNDLE
  return searchPaths;
#else
  // Add a executable related path
  auto tmpPath = F3DSystemTools::GetApplicationPath();
  tmpPath = tmpPath.parent_path().parent_path();
  tmpPath /= F3D::PluginsInstallDir;
  searchPaths.push_back(tmpPath.string());
  return searchPaths;
#endif
}
};

//----------------------------------------------------------------------------
void F3DPluginsTools::LoadPlugins(const std::vector<std::string>& plugins)
{
  try
  {
    f3d::engine::autoloadPlugins();

    for (const std::string& plugin : plugins)
    {
      if (!plugin.empty())
      {
        f3d::engine::loadPlugin(plugin, ::GetPluginSearchPaths());
      }
    }
  }
  catch (const f3d::engine::plugin_exception& e)
  {
    f3d::log::warn("Plugin failed to load: ", e.what());
  }
}

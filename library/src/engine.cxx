#include "engine.h"

#include "config.h"
#include "factory.h"
#include "init.h"
#include "interactor_impl.h"
#include "log.h"
#include "scene_impl.h"
#include "utils.h"
#include "window_impl.h"

#include "vtkF3DNoRenderWindow.h"

#include <vtkVersion.h>

#include <vtksys/DynamicLoader.hxx>
#include <vtksys/SystemTools.hxx>

#include <nlohmann/json.hpp>

namespace fs = std::filesystem;

namespace f3d
{
class engine::internals
{
public:
  template<typename F>
  static bool BackendAvailable(F&& func)
  {
    try
    {
      return func() != nullptr;
    }
    catch (const context::loading_exception&)
    {
      return false;
    }
  }

  std::unique_ptr<options> Options;
  std::unique_ptr<detail::window_impl> Window;
  std::unique_ptr<detail::scene_impl> Scene;
  std::unique_ptr<detail::interactor_impl> Interactor;
};

//----------------------------------------------------------------------------
engine::engine(
  const std::optional<window::Type>& windowType, bool offscreen, const context::function& loader)
  : Internals(new engine::internals)
{
  // Ensure all lib initialization is done (once)
  detail::init::initialize();

  fs::path cachePath;
#if defined(_WIN32)
  const char* appData = std::getenv("LOCALAPPDATA");
  if (appData && strlen(appData) > 0)
  {
    cachePath = fs::path(appData);
  }
#else

#if defined(__unix__)
  // Implementing XDG specifications
  const char* xdgCacheHome = std::getenv("XDG_CACHE_HOME");
  if (xdgCacheHome && strlen(xdgCacheHome) > 0)
  {
    cachePath = fs::path(xdgCacheHome);
  }
  else
#endif
  {
    const char* home = std::getenv("HOME");
    if (home && strlen(home) > 0)
    {
      cachePath = fs::path(home);
#if defined(__APPLE__)
      cachePath = cachePath / "Library" / "Caches";
#elif defined(__unix__)
      cachePath /= ".cache";
#endif
    }
  }
#endif
  if (cachePath.empty())
  {
    throw engine::cache_exception(
      "Could not setup cache, please set the appropriate environment variable");
  }
  cachePath /= "f3d";

  this->Internals->Options = std::make_unique<options>();

  this->Internals->Window =
    std::make_unique<detail::window_impl>(*this->Internals->Options, windowType, offscreen, loader);
  this->Internals->Window->SetCachePath(cachePath);

  this->Internals->Scene =
    std::make_unique<detail::scene_impl>(*this->Internals->Options, *this->Internals->Window);

  // Do not create an interactor for NONE or EXTERNAL
  if (windowType != window::Type::NONE && windowType != window::Type::EXTERNAL)
  {
    this->Internals->Interactor = std::make_unique<detail::interactor_impl>(
      *this->Internals->Options, *this->Internals->Window, *this->Internals->Scene);
  }
}

//----------------------------------------------------------------------------
engine engine::create(bool offscreen)
{
  return { std::nullopt, offscreen, nullptr };
}

//----------------------------------------------------------------------------
engine engine::createNone()
{
  return { window::Type::NONE, true, nullptr };
}

//----------------------------------------------------------------------------
engine engine::createGLX(bool offscreen)
{
  return { window::Type::GLX, offscreen, context::glx() };
}

//----------------------------------------------------------------------------
engine engine::createWGL(bool offscreen)
{
  return { window::Type::WGL, offscreen, context::wgl() };
}

//----------------------------------------------------------------------------
engine engine::createEGL()
{
  return { window::Type::EGL, true, context::egl() };
}

//----------------------------------------------------------------------------
engine engine::createOSMesa()
{
  return { window::Type::OSMESA, true, context::osmesa() };
}

//----------------------------------------------------------------------------
engine engine::createExternal(const context::function& getProcAddress)
{
  return { window::Type::EXTERNAL, false, getProcAddress };
}

//----------------------------------------------------------------------------
engine engine::createExternalGLX()
{
  return { window::Type::EXTERNAL, false, context::glx() };
}

//----------------------------------------------------------------------------
engine engine::createExternalWGL()
{
  return { window::Type::EXTERNAL, false, context::wgl() };
}

//----------------------------------------------------------------------------
engine engine::createExternalCOCOA()
{
  return { window::Type::EXTERNAL, false, context::cocoa() };
}

//----------------------------------------------------------------------------
engine engine::createExternalEGL()
{
  return { window::Type::EXTERNAL, false, context::egl() };
}

//----------------------------------------------------------------------------
engine engine::createExternalOSMesa()
{
  return { window::Type::EXTERNAL, false, context::osmesa() };
}

//----------------------------------------------------------------------------
engine::~engine()
{
  delete this->Internals;
}

//----------------------------------------------------------------------------
engine::engine(engine&& other) noexcept
{
  this->Internals = other.Internals;
  other.Internals = nullptr;
}

//----------------------------------------------------------------------------
engine& engine::operator=(engine&& other) noexcept
{
  delete this->Internals;
  this->Internals = other.Internals;
  other.Internals = nullptr;
  return *this;
}

//----------------------------------------------------------------------------
engine& engine::setOptions(const options& opt)
{
  *this->Internals->Options = opt;
  return *this;
}

//----------------------------------------------------------------------------
engine& engine::setOptions(options&& opt)
{
  *this->Internals->Options = std::move(opt);
  return *this;
}

//----------------------------------------------------------------------------
options& engine::getOptions()
{
  return *this->Internals->Options;
}

//----------------------------------------------------------------------------
window& engine::getWindow()
{
  if (this->Internals->Window->getType() == window::Type::NONE)
  {
    throw engine::no_window_exception("No window with this engine");
  }
  return *this->Internals->Window;
}

//----------------------------------------------------------------------------
scene& engine::getScene()
{
  return *this->Internals->Scene;
}

//----------------------------------------------------------------------------
interactor& engine::getInteractor()
{
  if (!this->Internals->Interactor)
  {
    throw engine::no_interactor_exception("No interactor with this engine");
  }
  return *this->Internals->Interactor;
}

//----------------------------------------------------------------------------
std::map<std::string, bool> engine::getRenderingBackendList()
{
  std::map<std::string, bool> backends;

  backends["glx"] = engine::internals::BackendAvailable(context::glx);
  backends["wgl"] = engine::internals::BackendAvailable(context::wgl);
  backends["cocoa"] = engine::internals::BackendAvailable(context::cocoa);
  backends["egl"] = engine::internals::BackendAvailable(context::egl);
  backends["osmesa"] = engine::internals::BackendAvailable(context::osmesa);

  return backends;
}

//----------------------------------------------------------------------------
void engine::loadPlugin(const std::string& pathOrName, const std::vector<fs::path>& searchPaths)
{
  if (pathOrName.empty())
  {
    return;
  }

  std::string pluginOrigin = "static";
  factory* factory = factory::instance();

  // check if the plugin is already loaded
  for (auto* plug : factory->getPlugins())
  {
    if (plug->getName() == pathOrName || plug->getOrigin() == pathOrName)
    {
      log::debug("Plugin \"", pathOrName, "\" already loaded");
      return;
    }
  }

  // check if the plugin is a known static plugin
  factory::plugin_initializer_t init_plugin = factory->getStaticInitializer(pathOrName);

  if (init_plugin == nullptr)
  {
    vtksys::DynamicLoader::LibraryHandle handle = nullptr;
    try
    {
      fs::path fullPath = utils::collapsePath(pathOrName);
      if (fs::exists(fullPath))
      {
        // plugin provided as full path
        log::debug("Trying to load plugin from: \"", fullPath, "\"");
        handle = vtksys::DynamicLoader::OpenLibrary(fullPath.string());

        if (!handle)
        {
          throw engine::plugin_exception("Cannot open the library \"" + fullPath.string() +
            "\": " + vtksys::DynamicLoader::LastError());
        }
        else
        {
          pluginOrigin = fullPath.string();
        }
      }
      else
      {
        // Not a full path
        // Construct the library file name from the plugin name
        std::string libName = vtksys::DynamicLoader::LibPrefix();
        libName += "f3d-plugin-";
        libName += pathOrName;
        libName += vtksys::DynamicLoader::LibExtension();

        // try search paths
        for (fs::path tryPath : searchPaths)
        {
          tryPath /= libName;
          if (fs::exists(tryPath))
          {
            log::debug(
              "Trying to load \"", pathOrName, "\" plugin from: \"", tryPath.string(), "\"");
            handle = vtksys::DynamicLoader::OpenLibrary(tryPath.string());

            if (handle)
            {
              // plugin is found and loaded
              pluginOrigin = tryPath.string();
              break;
            }
            else
            {
              log::debug("Could not load \"", tryPath.string(),
                "\" because: ", vtksys::DynamicLoader::LastError());
            }
          }
        }
        if (!handle)
        {
          // Rely on internal system (e.g. LD_LIBRARY_PATH on Linux) by giving only the file name
          log::debug("Trying to load plugin relying on internal system: ", libName);
          handle = vtksys::DynamicLoader::OpenLibrary(libName);
          if (!handle)
          {
            throw engine::plugin_exception("Cannot open the library \"" + pathOrName +
              "\": " + vtksys::DynamicLoader::LastError());
          }
          else
          {
            pluginOrigin = "system";
          }
        }
      }
    }
    catch (const fs::filesystem_error& ex)
    {
      throw engine::plugin_exception(std::string("Could not load plugin: ") + ex.what());
    }

    init_plugin = reinterpret_cast<factory::plugin_initializer_t>(
      vtksys::DynamicLoader::GetSymbolAddress(handle, "init_plugin"));
    if (init_plugin == nullptr)
    {
      throw engine::plugin_exception("Cannot find init_plugin symbol in library \"" + pathOrName +
        "\": " + vtksys::DynamicLoader::LastError());
    }
  }

  plugin* plug = init_plugin();
  plug->setOrigin(pluginOrigin);
  factory->load(plug);
  log::debug("Loaded plugin ", plug->getName(), " from: \"", plug->getOrigin(), "\"");
}

//----------------------------------------------------------------------------
void engine::autoloadPlugins()
{
  factory::instance()->autoload();
}

//----------------------------------------------------------------------------
std::vector<std::string> engine::getPluginsList(const fs::path& pluginPath)
{
  constexpr std::string_view ext = ".json";
  std::vector<std::string> pluginNames;
  try
  {
    for (auto& entry : fs::directory_iterator(pluginPath))
    {
      const fs::path& fullPath = entry.path();
      if (fullPath.extension() == ext)
      {
        try
        {
          auto root = nlohmann::json::parse(std::ifstream(fullPath));

          auto name = root.find("name");

          if (name != root.end() && name.value().is_string())
          {
            pluginNames.push_back(name.value().get<std::string>());
          }
        }
        catch (const nlohmann::json::parse_error& ex)
        {
          log::warn(fullPath, " is not a valid JSON file: ", ex.what());
        }
      }
    }
  }
  catch (const fs::filesystem_error&)
  {
  }

  return pluginNames;
}

//----------------------------------------------------------------------------
void engine::setPluginOption(const std::string& name, std::string value)
{
  std::map<std::string, std::string>& pluginOptions = factory::instance()->getPluginOptions();
  const auto iter = factory::instance()->getPluginOptions().find(name);
  if (iter == pluginOptions.end())
  {
    throw options::inexistent_exception("Plugin option " + std::string(name) + " does not exist");
  }
  iter->second = std::move(value);
}

//----------------------------------------------------------------------------
std::vector<std::string> engine::getAllPluginOptionNames()
{
  std::map<std::string, std::string>& pluginOptions = factory::instance()->getPluginOptions();
  std::vector<std::string> keys;
  keys.reserve(pluginOptions.size());
  for(const auto& [key, value] : pluginOptions) 
  {
    keys.push_back(key);
  }
  return keys;
}

//----------------------------------------------------------------------------
engine::libInformation engine::getLibInfo()
{
  libInformation libInfo;
  libInfo.Version = detail::LibVersion;
  libInfo.VersionFull = detail::LibVersionFull;
  libInfo.BuildDate = detail::LibBuildDate;
  libInfo.BuildSystem = detail::LibBuildSystem;
  libInfo.Compiler = detail::LibCompiler;

#if F3D_MODULE_RAYTRACING
  libInfo.Modules["Raytracing"] = true;
#else
  libInfo.Modules["Raytracing"] = false;
#endif

#if F3D_MODULE_EXR
  libInfo.Modules["OpenEXR"] = true;
#else
  libInfo.Modules["OpenEXR"] = false;
#endif

#if F3D_MODULE_UI
  libInfo.Modules["ImGui"] = true;
#else
  libInfo.Modules["ImGui"] = false;
#endif

  std::string vtkVersion = std::string(vtkVersion::GetVTKVersionFull());
  if (!vtkVersion.empty())
  {
    libInfo.VTKVersion = vtkVersion;
    std::string date = std::to_string(vtkVersion::GetVTKBuildVersion());
    if (date.size() == 8)
    {
      libInfo.VTKVersion += std::string(" (date: ") + date + ")";
    }
  }
  else
  {
    libInfo.VTKVersion = vtkVersion::GetVTKVersion();
  }

  libInfo.Copyrights.emplace_back("2019-2021 Kitware SAS");
  libInfo.Copyrights.emplace_back("2021-2025 Michael Migliore, Mathieu Westphal");
  libInfo.License = "BSD-3-Clause";

  return libInfo;
}

//----------------------------------------------------------------------------
std::vector<engine::readerInformation> engine::getReadersInfo()
{
  std::vector<readerInformation> readersInfo;
  const auto& plugins = factory::instance()->getPlugins();
  for (const auto& plugin : plugins)
  {
    for (const auto& reader : plugin->getReaders())
    {
      readerInformation info;
      info.PluginName = plugin->getName();
      info.Name = reader->getName();
      info.Description = reader->getLongDescription();
      info.Extensions = reader->getExtensions();
      info.MimeTypes = reader->getMimeTypes();
      info.HasSceneReader = reader->hasSceneReader();
      info.HasGeometryReader = reader->hasGeometryReader();
      readersInfo.push_back(info);
    }
  }
  return readersInfo;
}

//----------------------------------------------------------------------------
engine& engine::setCachePath(const fs::path& cachePath)
{
  this->Internals->Window->SetCachePath(cachePath);
  return *this;
}

//----------------------------------------------------------------------------
engine::no_window_exception::no_window_exception(const std::string& what)
  : exception(what)
{
}

//----------------------------------------------------------------------------
engine::no_interactor_exception::no_interactor_exception(const std::string& what)
  : exception(what)
{
}

//----------------------------------------------------------------------------
engine::plugin_exception::plugin_exception(const std::string& what)
  : exception(what)
{
}

//----------------------------------------------------------------------------
engine::cache_exception::cache_exception(const std::string& what)
  : exception(what)
{
}

}

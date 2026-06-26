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

#include <fstream>

namespace fs = std::filesystem;

namespace
{
//----------------------------------------------------------------------------
nlohmann::ordered_json CaptureState(f3d::engine& eng, const fs::path& baseDir)
{
  nlohmann::ordered_json root;

  // Store the added files, relative to the statefile directory (baseDir) when the file actually
  // lives under it, so that the statefile stays portable if moved alongside its files. Files
  // outside baseDir (the relative path would escape it with "..") are stored as absolute paths:
  // such a relative path is not portable anyway and would break when loaded without a baseDir
  // (e.g. from the standard input or the clipboard). baseDir is empty when serializing to a
  // string, in which case absolute paths are always used.
  nlohmann::ordered_json files = nlohmann::ordered_json::array();
  for (const fs::path& file : eng.getScene().getAddedFiles())
  {
    fs::path stored = fs::absolute(file);
    if (!baseDir.empty())
    {
      const fs::path rel = fs::relative(fs::absolute(file), fs::absolute(baseDir));
      if (!rel.empty() && *rel.begin() != "..")
      {
        stored = rel;
      }
    }
    files.push_back(stored.generic_string());
  }
  if (files.empty())
  {
    f3d::log::info("No files to save in statefile");
  }
  root["files"] = files;

  // Camera, only if a window is available
  try
  {
    const f3d::camera_state_t state = eng.getWindow().getCamera().getState();
    nlohmann::ordered_json camera;
    camera["position"] = { state.position[0], state.position[1], state.position[2] };
    camera["focal_point"] = { state.focalPoint[0], state.focalPoint[1], state.focalPoint[2] };
    camera["view_up"] = { state.viewUp[0], state.viewUp[1], state.viewUp[2] };
    camera["view_angle"] = state.viewAngle;
    root["camera"] = camera;
  }
  catch (const f3d::engine::no_window_exception&)
  {
    // No window available, nothing to save for the camera
  }

  nlohmann::ordered_json options = nlohmann::ordered_json::object();
  const f3d::options& opts = eng.getOptions();
  for (const std::string& name : opts.getNames())
  {
    options[name] = opts.getAsString(name);
  }
  root["options"] = options;

  return root;
}

//----------------------------------------------------------------------------
void RestoreState(f3d::engine& eng, const nlohmann::ordered_json& root, const fs::path& baseDir)
{
  // Clear the scene first so that the statefile fully replaces the current state
  f3d::scene& sce = eng.getScene();
  sce.clear();

  if (root.contains("options"))
  {
    f3d::options& opts = eng.getOptions();
    for (const auto& [name, value] : root.at("options").items())
    {
      try
      {
        opts.setAsString(name, value.get<std::string>());
      }
      catch (const f3d::options::inexistent_exception&)
      {
        f3d::log::warn("Statefile option \"", name, "\" does not exist, skipping it");
      }
      catch (const f3d::options::parsing_exception&)
      {
        f3d::log::warn("Statefile option \"", name, "\" could not be parsed from value \"",
          value.get<std::string>(), "\", skipping it");
      }
    }
  }
  else
  {
    f3d::log::warn("No options found in statefile");
  }

  // Add the saved files, resolving relative paths against the statefile directory (baseDir),
  // mirroring how CaptureState stored them
  if (root.contains("files"))
  {
    std::vector<fs::path> files;
    for (const auto& file : root.at("files"))
    {
      fs::path path = file.get<std::string>();
      if (path.is_relative() && !baseDir.empty())
      {
        path = (baseDir / path).lexically_normal();
      }
      files.emplace_back(path);
    }
    if (!files.empty())
    {
      // Let any scene::load_failure_exception propagate to the caller: a statefile that cannot
      // reload its files is a failure to restore the state, not something to silently ignore.
      sce.add(files);
    }
  }

  if (root.contains("camera"))
  {
    try
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
      eng.getWindow().getCamera().setState(state);
    }
    catch (const f3d::engine::no_window_exception&)
    {
      f3d::log::info("No window available, skipping camera state from statefile");
    }
  }
}
}

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

  // Recover cache directory
  fs::path cachePath;
#if defined(_WIN32)
  std::optional<std::string> appData = utils::getKnownFolder(utils::KnownFolder::LOCALAPPDATA);
  if (appData.has_value() && !appData.value().empty())
  {
    cachePath = fs::path(appData.value());
  }
#else

#if defined(__unix__)
  // Implementing XDG specifications
  std::optional<std::string> xdgCacheHome = utils::getEnv("XDG_CACHE_HOME");
  if (xdgCacheHome.has_value() && !xdgCacheHome.value().empty())
  {
    cachePath = fs::path(xdgCacheHome.value());
  }
  else
#endif
  {
    std::optional<std::string> home = utils::getEnv("HOME");
    if (home.has_value() && !home.value().empty())
    {
      cachePath = fs::path(home.value());
#if defined(__APPLE__)
      cachePath = cachePath / "Library" / "Caches";
#elif defined(__unix__)
      cachePath /= ".cache";
#endif
    }
  }
#endif

  this->Internals->Options = std::make_unique<options>();

  this->Internals->Window =
    std::make_unique<detail::window_impl>(*this->Internals->Options, windowType, offscreen, loader);

  if (!cachePath.empty())
  {
    cachePath /= "f3d";
    this->Internals->Window->SetCachePath(cachePath);
  }

  this->Internals->Scene =
    std::make_unique<detail::scene_impl>(*this->Internals->Options, *this->Internals->Window);

  // Do not create an interactor for NONE
  if (windowType != window::Type::NONE)
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
engine engine::createExternal([[maybe_unused]] const context::function& getProcAddress)
{
  if (getProcAddress == nullptr)
  {
    throw engine::no_window_exception(
      "Cannot create an external window without a context function");
  }
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
engine& engine::saveStatefile(const fs::path& statefilePath)
{
  try
  {
    std::ofstream stream(statefilePath);
    if (!stream.is_open())
    {
      throw engine::statefile_exception(
        "Could not open statefile for writing: " + statefilePath.string());
    }
    stream << ::CaptureState(*this, statefilePath.parent_path()).dump(2) << '\n';
  }
  catch (const fs::filesystem_error& ex)
  {
    throw engine::statefile_exception(
      "Could not save statefile " + statefilePath.string() + ": " + ex.what());
  }
  return *this;
}

//----------------------------------------------------------------------------
engine& engine::loadStatefile(const fs::path& statefilePath)
{
  try
  {
    std::ifstream stream(statefilePath);
    if (!stream.is_open())
    {
      throw engine::statefile_exception(
        "Could not open statefile for reading: " + statefilePath.string());
    }
    nlohmann::ordered_json root = nlohmann::ordered_json::parse(stream);
    ::RestoreState(*this, root, statefilePath.parent_path());
  }
  catch (const nlohmann::json::exception& ex)
  {
    throw engine::statefile_exception(
      "Could not parse statefile " + statefilePath.string() + ": " + ex.what());
  }
  catch (const fs::filesystem_error& ex)
  {
    throw engine::statefile_exception(
      "Could not load statefile " + statefilePath.string() + ": " + ex.what());
  }
  return *this;
}

//----------------------------------------------------------------------------
std::string engine::saveStatefileToString()
{
  try
  {
    return ::CaptureState(*this, {}).dump(2);
  }
  catch (const fs::filesystem_error& ex)
  {
    throw engine::statefile_exception(
      std::string("Could not save statefile to string: ") + ex.what());
  }
}

//----------------------------------------------------------------------------
engine& engine::loadStatefileFromString(const std::string& statefileContent)
{
  try
  {
    nlohmann::ordered_json root = nlohmann::ordered_json::parse(statefileContent);
    ::RestoreState(*this, root, {});
  }
  catch (const nlohmann::json::exception& ex)
  {
    throw engine::statefile_exception(
      std::string("Could not parse statefile content: ") + ex.what());
  }
  return *this;
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

  // For easier removal when removing deprecation: F3D_DEPRECATED
  if (pathOrName == "exodus")
  {
    f3d::log::warn("The 'exodus' plugin is deprecated, load 'hdf' instead");
    return f3d::engine::loadPlugin("hdf", searchPaths);
  }

  std::string pluginOrigin = "static";
  factory* factory = factory::instance();

  // check if the plugin is already loaded
  auto plugs = factory->getPlugins();
  if (std::ranges::any_of(plugs, [pathOrName](const plugin* plug)
        { return (plug->getName() == pathOrName || plug->getOrigin() == pathOrName); }))
  {
    log::debug("Plugin \"", pathOrName, "\" already loaded");
    return;
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
std::vector<std::string> engine::getAllReaderOptionNames()
{
  return factory::instance()->getAllReaderOptionNames();
}

//----------------------------------------------------------------------------
void engine::setReaderOption(const std::string& name, const std::string& value)
{
  if (!factory::instance()->setReaderOption(name, value))
  {
    throw options::inexistent_exception("Reader option " + name + " does not exist");
  }
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

#if F3D_MODULE_WEBP
  libInfo.Modules["WebP"] = true;
#else
  libInfo.Modules["WebP"] = false;
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
  libInfo.Copyrights.emplace_back("2025 F3D-APP Foundation");
  libInfo.License = "BSD-3-Clause";

  return libInfo;
}

//----------------------------------------------------------------------------
std::vector<engine::readerInformation> engine::getReadersInfo()
{
  std::vector<readerInformation> readersInfo;
  const auto& plugins = factory::instance()->getPlugins();
  for (const auto* plugin : plugins)
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
      info.SupportsStream = reader->supportsStream();
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

//----------------------------------------------------------------------------
engine::statefile_exception::statefile_exception(const std::string& what)
  : exception(what)
{
}

}

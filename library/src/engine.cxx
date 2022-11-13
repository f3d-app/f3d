#include "engine.h"

#include "config.h"
#include "init.h" // This includes is needed on mac so that the global variable is created
#include "interactor_impl.h"
#include "loader_impl.h"
#include "log.h"
#include "window_impl.h"

#include "factory.h"

#include "vtkF3DConfigure.h"
#include "vtkF3DNoRenderWindow.h"

#include <vtkVersion.h>

#include <vtksys/DynamicLoader.hxx>
#include <vtksys/Encoding.hxx>
#include <vtksys/SystemTools.hxx>

namespace f3d
{
class engine::internals
{
public:
  std::unique_ptr<options> Options;
  std::unique_ptr<detail::window_impl> Window;
  std::unique_ptr<detail::loader_impl> Loader;
  std::unique_ptr<detail::interactor_impl> Interactor;
};

//----------------------------------------------------------------------------
engine::engine(window::Type windowType)
  : Internals(new engine::internals)
{
  this->Internals->Options = std::make_unique<options>();

  this->Internals->Window =
    std::make_unique<detail::window_impl>(*this->Internals->Options, windowType);

  this->Internals->Loader =
    std::make_unique<detail::loader_impl>(*this->Internals->Options, *this->Internals->Window);

  // Do not create an interactor for NONE or EXTERNAL
  if (windowType != window::Type::NONE && windowType != window::Type::EXTERNAL)
  {
    this->Internals->Interactor = std::make_unique<detail::interactor_impl>(
      *this->Internals->Options, *this->Internals->Window, *this->Internals->Loader);
  }
}

//----------------------------------------------------------------------------
engine::~engine()
{
  delete this->Internals;
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
loader& engine::getLoader()
{
  return *this->Internals->Loader;
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
void engine::loadPlugin(const std::string& path)
{
  // check if the plugin is a known static plugin
  factory* factory = factory::instance();
  factory::plugin_initializer_t init_plugin = factory->getStaticInitializer(path);

  if (init_plugin == nullptr)
  {
    // try if the path is a library full path
    vtksys::DynamicLoader::LibraryHandle handle = vtksys::DynamicLoader::OpenLibrary(path);
    if (!handle)
    {
      std::string fullPath;

      // Right now, plugins should be located in the same folder than f3d.exe on Windows
      // On Linux/macOS, when we are not using a full path, we rely on LD_LIBRARY_PATH
#ifdef _WIN32
      std::vector<wchar_t> pathBuf(40000);
      if (GetModuleFileNameW(
            GetModuleHandle(nullptr), pathBuf.data(), static_cast<DWORD>(pathBuf.size())))
      {
        std::string progPath =
          vtksys::SystemTools::GetProgramPath(vtksys::Encoding::ToNarrow(pathBuf.data()));
        fullPath = vtksys::SystemTools::ConvertToWindowsOutputPath(progPath) + "\\";
      }
#endif

      // construct the library file name from the plugin name
      fullPath += vtksys::DynamicLoader::LibPrefix();
      fullPath += "f3d-plugin-";
      fullPath += path;
      fullPath += vtksys::DynamicLoader::LibExtension();

      handle = vtksys::DynamicLoader::OpenLibrary(fullPath);
      if (!handle)
      {
        throw engine::plugin_exception(
          "Cannot open the library \"" + path + "\": " + vtksys::DynamicLoader::LastError());
      }
    }

    init_plugin = reinterpret_cast<factory::plugin_initializer_t>(
      vtksys::DynamicLoader::GetSymbolAddress(handle, "init_plugin"));
    if (init_plugin == nullptr)
    {
      throw engine::plugin_exception("Cannot find init_plugin symbol in library \"" + path +
        "\": " + vtksys::DynamicLoader::LastError());
    }
  }

  plugin* p = init_plugin();

  factory->load(p);
}

//----------------------------------------------------------------------------
void engine::autoloadPlugins()
{
  factory::instance()->autoload();
}

//----------------------------------------------------------------------------
engine::libInformation engine::getLibInfo()
{
  libInformation libInfo;
  libInfo.Version = detail::LibVersion;
  libInfo.BuildDate = detail::LibBuildDate;
  libInfo.BuildSystem = detail::LibBuildSystem;
  libInfo.Compiler = detail::LibCompiler;

  std::string tmp;

#if F3D_MODULE_RAYTRACING
  tmp = "ON";
#else
  tmp = "OFF";
#endif
  libInfo.RaytracingModule = tmp;

#if F3D_MODULE_EXTERNAL_RENDERING
  tmp = "ON";
#else
  tmp = "OFF";
#endif
  libInfo.ExternalRenderingModule = tmp;

  libInfo.VTKVersion = std::string(VTK_VERSION) + std::string(" (build ") +
    std::to_string(VTK_BUILD_VERSION) + std::string(")");

  libInfo.PreviousCopyright = "Copyright (C) 2019-2021 Kitware SAS";
  libInfo.Copyright = "Copyright (C) 2021-2022 Michael Migliore, Mathieu Westphal";
  libInfo.License = "BSD-3-Clause";
  libInfo.Authors = "Michael Migliore, Mathieu Westphal and Joachim Pouderoux";

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
      readersInfo.push_back(info);
    }
  }
  return readersInfo;
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

}

#include "f3d_engine.h"

#include "f3d_config.h"
#include "f3d_interactor_impl.h"
#include "f3d_loader_impl.h"
#include "f3d_log.h"
#include "f3d_options.h"
#include "f3d_window_impl_noRender.h"
#include "f3d_window_impl_standard.h"

#include <vtkVersion.h>

namespace f3d
{
class engine::F3DInternals
{
public:
  std::unique_ptr<options> Options;
  std::unique_ptr<window_impl> Window;
  std::unique_ptr<loader_impl> Loader;
  std::unique_ptr<interactor_impl> Interactor;
};

//----------------------------------------------------------------------------
engine::engine(WindowTypeEnum windowType, bool offscreen)
  : Internals(new engine::F3DInternals())
  , WindowType(windowType)
  , Offscreen(offscreen)
{
#if NDEBUG
  vtkObject::GlobalWarningDisplayOff();
#endif

  // Disable vtkLogger in case VTK was built with log support
  vtkLogger::SetStderrVerbosity(vtkLogger::VERBOSITY_OFF);
#if VTK_VERSION_NUMBER >= VTK_VERSION_CHECK(9, 0, 20200701)
  vtkLogger::SetInternalVerbosityLevel(vtkLogger::VERBOSITY_OFF);
#endif

  // instanciate our own polydata mapper and output windows
  vtkNew<vtkF3DObjectFactory> factory;
  vtkObjectFactory::RegisterFactory(factory);
  vtkObjectFactory::SetAllEnableFlags(0, "vtkPolyDataMapper", "vtkOpenGLPolyDataMapper");

  // Make sure to initialize the output window
  // after the object factory and before the first usage.
  f3d::log::setQuiet(false);
}

//----------------------------------------------------------------------------
engine::~engine() = default;

//----------------------------------------------------------------------------
options& engine::getOptions()
{
  if (!this->Internals->Options)
  {
    this->Internals->Options = std::make_unique<options>();
  }
  return *this->Internals->Options;
}

//----------------------------------------------------------------------------
window& engine::getWindow()
{
  if (!this->Internals->Window)
  {
    switch (this->WindowType)
    {
      case (engine::WindowTypeEnum::WINDOW_NO_RENDER):
        this->Internals->Window = std::make_unique<window_impl_noRender>(this->getOptions());
        break;
      case (engine::WindowTypeEnum::WINDOW_STANDARD):
      default:
        this->Internals->Window =
          std::make_unique<window_impl_standard>(this->getOptions(), this->Offscreen);
        break;
    }
  }
  return *this->Internals->Window;
}

//----------------------------------------------------------------------------
loader& engine::getLoader()
{
  if (!this->Internals->Loader)
  {
    this->Internals->Loader = std::make_unique<loader_impl>(
      this->getOptions(), static_cast<window_impl&>(this->getWindow()));
  }
  return *this->Internals->Loader;
}

//----------------------------------------------------------------------------
interactor& engine::getInteractor()
{
  if (!this->Internals->Interactor)
  {
    this->Internals->Interactor = std::make_unique<interactor_impl>(this->getOptions(),
      static_cast<window_impl&>(this->getWindow()), static_cast<loader_impl&>(this->getLoader()));
  }
  return *this->Internals->Interactor;
}

//----------------------------------------------------------------------------
void engine::printVersion()
{
  // TODO engin should help with crafting the version string but should not be responsible for the printing itself, to improve 
  std::string version = f3d::AppName + " " + f3d::AppVersion + "\n\n";

  version += f3d::AppTitle;
  version += "\nVersion: ";
  version += f3d::AppVersion;
  version += "\nBuild date: ";
  version += f3d::AppBuildDate;
  version += "\nSystem: ";
  version += f3d::AppBuildSystem;
  version += "\nCompiler: ";
  version += f3d::AppCompiler;
  version += "\nRayTracing module: ";
#if F3D_MODULE_RAYTRACING
  version += "ON";
#else
  version += "OFF";
#endif
  version += "\nExodus module: ";
#if F3D_MODULE_EXODUS
  version += "ON";
#else
  version += "OFF";
#endif
  version += "\nOpenCASCADE module: ";
#if F3D_MODULE_OCCT
  version += F3D_OCCT_VERSION;
#if F3D_MODULE_OCCT_XCAF
  version += " (full support)";
#else
  version += " (no metadata)";
#endif
#else
  version += "OFF";
#endif
  version += "\nAssimp module: ";
#if F3D_MODULE_ASSIMP
  version += F3D_ASSIMP_VERSION;
#else
  version += "OFF";
#endif
  version += "\nVTK version: ";
  version += std::string(VTK_VERSION) + std::string(" (build ") +
    std::to_string(VTK_BUILD_VERSION) + std::string(")");

  version += "\n\nCopyright (C) 2019-2021 Kitware SAS.";
  version += "\nCopyright (C) 2021-2022 Michael Migliore, Mathieu Westphal.";
  version += "\nLicense BSD-3-Clause.";
  version += "\nWritten by Michael Migliore, Mathieu Westphal and Joachim Pouderoux.";

  f3d::log::setUseColoring(false);
  f3d::log::info(version);
  f3d::log::setUseColoring(true);
  f3d::log::waitForUser();
}
}

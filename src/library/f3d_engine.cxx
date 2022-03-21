#include "f3d_engine.h"

#include "f3d_interactor_impl.h"
#include "f3d_loader_impl.h"
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
}

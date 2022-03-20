#include "f3d_engine.h"

#include "f3d_interactor_impl.h"
#include "f3d_loader_impl.h"
#include "f3d_options.h"
#include "f3d_windowNoRender.h"
#include "f3d_windowStandard.h"

namespace f3d
{
class engine::F3DInternals
{
public:
  std::unique_ptr<options> Options;
  std::unique_ptr<window> Window;
  std::unique_ptr<loader_impl> Loader;
  std::unique_ptr<interactor_impl> Interactor;
};

//----------------------------------------------------------------------------
engine::engine(WindowTypeEnum windowType, bool offscreen)
  : Internals(new engine::F3DInternals())
  , WindowType(windowType)
  , Offscreen(offscreen)
{
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
        this->Internals->Window = std::make_unique<windowNoRender>(this->getOptions());
        break;
      case (engine::WindowTypeEnum::WINDOW_STANDARD):
      default:
        this->Internals->Window = std::make_unique<windowStandard>(this->getOptions(), this->Offscreen);
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
    this->Internals->Loader = std::make_unique<loader_impl>(this->getOptions(), this->getWindow());
  }
  return *this->Internals->Loader;
}

//----------------------------------------------------------------------------
interactor& engine::getInteractor()
{
  if (!this->Internals->Interactor)
  {
    this->Internals->Interactor =
      std::make_unique<interactor_impl>(this->getOptions(), this->getWindow(), static_cast<loader_impl&>(this->getLoader()));
  }
  return *this->Internals->Interactor;
}
}

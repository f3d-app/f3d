#include "f3d_engine.h"

#include "f3d_interactor.h"
#include "f3d_loader.h"
#include "f3d_options.h"
#include "f3d_windowNoRender.h"
#include "f3d_windowStandard.h"

namespace f3d
{
//----------------------------------------------------------------------------
engine::engine(WindowTypeEnum windowType, bool offscreen)
  : WindowType(windowType)
  , Offscreen(offscreen)
{
}

//----------------------------------------------------------------------------
options& engine::getOptions()
{
  if (!this->Options)
  {
    this->Options = std::make_unique<options>();
  }
  return *this->Options;
}

//----------------------------------------------------------------------------
window& engine::getWindow()
{
  if (!this->Window)
  {
    switch (this->WindowType)
    {
      case (engine::WindowTypeEnum::WINDOW_NO_RENDER):
        this->Window = std::make_unique<windowNoRender>(this->getOptions());
        break;
      case (engine::WindowTypeEnum::WINDOW_STANDARD):
      default:
        this->Window = std::make_unique<windowStandard>(this->getOptions(), this->Offscreen);
        break;
    }
  }
  return *this->Window;
}

//----------------------------------------------------------------------------
loader& engine::getLoader()
{
  if (!this->Loader)
  {
    this->Loader = std::make_unique<loader>(this->getOptions(), this->getWindow());
  }
  return *this->Loader;
}

//----------------------------------------------------------------------------
interactor& engine::getInteractor()
{
  if (!this->Interactor)
  {
    this->Interactor =
      std::make_unique<interactor>(this->getOptions(), this->getWindow(), this->getLoader());
  }
  return *this->Interactor;
}
}

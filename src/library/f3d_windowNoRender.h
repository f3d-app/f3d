#ifndef f3d_windowNoRender_h
#define f3d_windowNoRender_h

#include "f3d_window.h"

#include <memory>
#include <string>

class vtkRenderWindow;
class vtkF3DRenderer;
namespace f3d
{
class loader;
class windowNoRender : public window
{
public:
  windowNoRender(const options& options);
  ~windowNoRender() override;

  // TODO Private API
  vtkRenderWindow* GetRenderWindow() override;

private:
  class F3DInternals;
  std::unique_ptr<F3DInternals> Internals;
};
}

#endif

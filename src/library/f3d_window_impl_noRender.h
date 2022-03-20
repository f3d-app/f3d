#ifndef f3d_window_impl_noRender_h
#define f3d_window_impl_noRender_h

#include "f3d_window_impl.h"

#include <memory>
#include <string>

// TODO Doc
class vtkRenderWindow;
class vtkF3DRenderer;
namespace f3d
{
class loader;
class window_impl_noRender : public window_impl
{
public:
  window_impl_noRender(const options& options);
  ~window_impl_noRender() override;

  vtkRenderWindow* GetRenderWindow() override;

private:
  class F3DInternals;
  std::unique_ptr<F3DInternals> Internals;
};
}

#endif

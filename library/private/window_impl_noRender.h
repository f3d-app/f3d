/**
 * @class   window_impl
 * @brief   A concrete implementation of window without rendering capabilities
 *
 * A concrete implementation of window that only provide a vtkRenderWindow
 * that do not support rendering as it is required by the vtkImporter API
 * See window.h and window_impl.h for the class documentation
 */

#ifndef f3d_window_impl_noRender_h
#define f3d_window_impl_noRender_h

#include "window_impl.h"

#include <memory>
#include <string>

class vtkRenderWindow;
class vtkF3DRenderer;
namespace f3d
{
class loader;

namespace detail
{
class window_impl_noRender : public window_impl
{
public:
  /**
   * Create the internal vtkRenderWindow as offscreen
   * so it does not show on screen.
   */
  explicit window_impl_noRender(const options& options);

  /**
   * Default destructor
   */
  ~window_impl_noRender() override;

  /**
   * Documented private API
   */
  vtkRenderWindow* GetRenderWindow() override;

private:
  class internals;
  std::unique_ptr<internals> Internals;
};
}
}

#endif

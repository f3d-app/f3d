/**
 * @class   window_impl
 * @brief   A concrete implementation of window
 *
 * A concrete implementation of window that provide a vtkRenderWindow
 * to render into.
 * See window.h and window_impl.h for the class documentation
 */

#ifndef f3d_window_impl_standard_h
#define f3d_window_impl_standard_h

#include "window_impl.h"

#include <memory>
#include <string>

class vtkRenderWindow;
class vtkF3DGenericImporter;
namespace f3d
{
class loader;

namespace detail
{
class window_impl_standard : public window_impl
{
public:
  /**
   * Create the internal vtkRenderWindow using the offscreen param
   * and store option ref for later usage
   */
  window_impl_standard(const options& options, bool offscreen);

  /**
   * Default destructor
   */
  ~window_impl_standard() override;

  //@{
  /**
   * Documented public API
   */
  bool update() override;
  bool render() override;
  image renderToImage(bool noBackground = false) override;
  bool setIcon(const void* icon, size_t iconSize) override;
  bool setWindowName(const std::string& windowName) override;
  //@}

  //@{
  /**
   * Documented private API
   */
  void Initialize(bool withColoring, std::string fileInfo) override;
  void InitializeRendererWithColoring(vtkF3DGenericImporter* importer) override;
  vtkRenderWindow* GetRenderWindow() override;
  //@}

private:
  class internals;
  std::unique_ptr<internals> Internals;
};
}
}

#endif

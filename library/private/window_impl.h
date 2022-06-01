/**
 * @class   window_impl
 * @brief   A private abstract implementation of window
 *
 * A private abstract implementation of window that defines the private API
 * See window.h for the class documentation
 */

#ifndef f3d_window_impl_h
#define f3d_window_impl_h

#include "window.h"

class vtkRenderWindow;
class vtkF3DGenericImporter;
namespace f3d
{
class options;

namespace detail
{
class window_impl : public window
{
public:
  ~window_impl() override = default;

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

  /**
   * Implementation only API.
   * Create and initialize the internal vtkF3DRenderer with the provided parameters
   * Called by the loader right before reading a file
   */
  virtual void Initialize(bool withColoring, std::string fileInfo);

  /**
   * Implementation only API.
   * Initialize an already created vtkF3DRendererWithColoring with the provided importer
   * Called by the loader right after reading a file
   */
  virtual void InitializeRendererWithColoring(vtkF3DGenericImporter* importer);

  /**
   * Implementation only API.
   * Get a pointer to the internal vtkRenderWindow
   */
  virtual vtkRenderWindow* GetRenderWindow() = 0;

protected:
  window_impl(const options&);

  const options& Options;
};
}
}

#endif

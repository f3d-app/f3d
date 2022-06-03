/**
 * @class   window
 * @brief   A concrete implementation of window
 *
 * A concrete implementation of window that provide a vtkRenderWindow
 * to render into.
 * See window.h for the class documentation
 */

#ifndef f3d_window_impl_h
#define f3d_window_impl_h

#include "window.h"

#include <memory>
#include <string>

class vtkRenderWindow;
class vtkF3DGenericImporter;
namespace f3d
{
class loader;
class options;

namespace detail
{
class window_impl : public window
{
public:
  /**
   * Enumeration of valid standard window type
   */
  enum class WindowType : unsigned char
  {
    NO_RENDER,
    NATIVE,
    NATIVE_OFFSCREEN,
    EXTERNAL
  };

  /**
   * Create the internal vtkRenderWindow using the offscreen param
   * and store option ref for later usage
   */
  window_impl(const options& options, WindowType type);

  /**
   * Default destructor
   */
  ~window_impl() override;

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
  virtual vtkRenderWindow* GetRenderWindow();

private:
  class internals;
  std::unique_ptr<internals> Internals;
};
}
}

#endif

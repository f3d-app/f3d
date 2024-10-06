/**
 * @class   window_impl
 * @brief   A concrete implementation of window
 *
 * A concrete implementation of window that can provide a window to render into.
 * It also defined implementation only API used by the libf3d.
 * See window.h for the class documentation
 */

#ifndef f3d_window_impl_h
#define f3d_window_impl_h

#include "log.h"
#include "window.h"

#include <memory>

class vtkRenderWindow;
class vtkF3DMetaImporter;
namespace f3d
{
class options;

namespace detail
{
class window_impl : public window
{
public:
  /**
   * Create the internal vtkRenderWindow using the offscreen param
   * and store option ref for later usage
   */
  window_impl(const options& options, Type type);

  /**
   * Default destructor
   */
  ~window_impl() override;

  ///@{
  /**
   * Documented public API
   */
  Type getType() override;
  camera& getCamera() override;
  bool render() override;
  image renderToImage(bool noBackground = false) override;
  int getWidth() const override;
  int getHeight() const override;
  window& setAnimationNameInfo(const std::string& name);
  window& setSize(int width, int height) override;
  window& setPosition(int x, int y) override;
  window& setIcon(const unsigned char* icon, size_t iconSize) override;
  window& setWindowName(const std::string& windowName) override;
  point3_t getWorldFromDisplay(const point3_t& displayPoint) const override;
  point3_t getDisplayFromWorld(const point3_t& worldPoint) const override;
  ///@}

  /**
   * Implementation only API.
   * Initialize the renderer by clearing it of all actors.
   */
  void Initialize();

  /**
   * Implementation only API.
   * Initialize the up vector on the renderer using the Up string option
   */
  void InitializeUpVector();

  /**
   * Implementation only API.
   * Set the importer on the internal renderer
   */
  void SetImporter(vtkF3DMetaImporter* importer);

  /**
   * Implementation only API.
   * Use all the rendering related options to update the configuration of the window
   * and the rendering stack below.
   * This will be called automatically when calling loader::loadFile but can also be called manually
   * when needed. This must be called, either manually or automatically, before any render call.
   * Return true on success, false otherwise.
   */
  void UpdateDynamicOptions();

  /**
   * Implementation only API.
   * Print scene description to log using provided verbose level
   */
  void PrintSceneDescription(log::VerboseLevel level);

  /**
   * Implementation only API.
   * Print coloring description to log using provided verbose level if available
   */
  void PrintColoringDescription(log::VerboseLevel level);

  /**
   * Implementation only API.
   * Get a pointer to the internal vtkRenderWindow
   */
  virtual vtkRenderWindow* GetRenderWindow();

  /**
   * Implementation only API.
   * Set the cache path.
   */
  void SetCachePath(const std::string& cachePath);

private:
  class internals;
  std::unique_ptr<internals> Internals;
};
}
}

#endif

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

#include "context.h"
#include "interactor_impl.h"
#include "log.h"
#include "window.h"

#include <filesystem>
#include <memory>
#include <optional>

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
  window_impl(const options& options, const std::optional<Type>& type, bool offscreen,
    const context::function& getProcAddress);
  /**
   * Default destructor
   */
  ~window_impl() override;

  ///@{
  /**
   * Documented public API
   */
  Type getType() override;
  bool isOffscreen() override;
  camera& getCamera() override;
  bool render() override;
  image renderToImage(bool noBackground = false) override;
  int getWidth() const override;
  int getHeight() const override;
  window& setSize(int width, int height) override;
  window& setPosition(int x, int y) override;
  window& setIcon(const unsigned char* icon, size_t iconSize) override;
  window& setWindowName(std::string_view windowName) override;
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
   * This is called automatically when calling scene::add and window::render but can also be called
   * manually when needed. Return true on success, false otherwise.
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
  void SetCachePath(const std::filesystem::path& cachePath);

  /**
   * Implementation only API.
   * Set the interactor to use when recovering bindings documentation.
   */
  void SetInteractor(interactor_impl* interactor);

  /**
   * Trigger a render only of the UI
   * Does nothing if F3D_MODULE_UI is OFF
   */
  void RenderUIOnly();

private:
  class internals;
  std::unique_ptr<internals> Internals;
};
}
}

#endif

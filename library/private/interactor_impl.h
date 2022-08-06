/**
 * @class   interactor_impl
 * @brief   A concrete implementation of interactor
 *
 * A concrete implementation of interactor that hides the private API
 * See interactor.h for the class documentation
 */

#ifndef f3d_interactor_impl_h
#define f3d_interactor_impl_h

#include "interactor.h"

#include <memory>

class vtkInteractorObserver;
class vtkImporter;
namespace f3d
{
class options;

namespace detail
{
class loader_impl;
class window_impl;

class interactor_impl : public interactor
{
public:
  //@{
  /**
   * Documented public API
   */
  interactor_impl(options& options, window_impl& window, loader_impl& loader) noexcept;
  ~interactor_impl() noexcept;

  interactor& setKeyPressCallBack(std::function<bool(int, std::string)> callBack) noexcept override;
  interactor& setDropFilesCallBack(
    std::function<bool(std::vector<std::string>)> callBack) noexcept override;

  unsigned long createTimerCallBack(double time, std::function<void()> callBack) noexcept override;
  void removeTimerCallBack(unsigned long id) noexcept override;

  void toggleAnimation() noexcept override;
  void startAnimation() noexcept override;
  void stopAnimation() noexcept override;
  bool isPlayingAnimation() noexcept override;

  void enableCameraMovement() noexcept override;
  void disableCameraMovement() noexcept override;

  bool playInteraction(const std::string& file) noexcept override;
  bool recordInteraction(const std::string& file) noexcept override;

  void start() noexcept override;
  void stop() noexcept override;
  //@}

  /**
   * Implementation only API.
   * An utility method to set internal VTK interactor on a vtkInteractorObserver object.
   */
  void SetInteractorOn(vtkInteractorObserver* observer);

  /**
   * Implementation only API.
   * Initialize the animation manager using interactor objects.
   * This is called by the loader after loading a file.
   */
  void InitializeAnimation(vtkImporter* importer);

private:
  class internals;
  std::unique_ptr<internals> Internals;
};
}
}

#endif

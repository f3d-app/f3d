#ifndef f3d_interactor_impl_h
#define f3d_interactor_impl_h

#include "f3d_interactor.h"

#include <memory>

class vtkInteractorObserver;
class vtkImporter;
namespace f3d
{
class options;
class loader;
class window;
class interactor_impl : public interactor
{
public:
  interactor_impl(const options& options, window& window, loader& loader);
  ~interactor_impl();

  void setKeyPressCallBack(std::function<bool(int, std::string)> callBack) override;
  void setDropFilesCallBack(std::function<bool(std::vector<std::string>)> callBack) override;

  unsigned long createTimerCallBack(double time, std::function<void()> callBack) override;
  void removeTimerCallBack(unsigned long id) override;

  //@{
  /**
   * Control the animation
   */
  void toggleAnimation() override;
  void startAnimation() override;
  void stopAnimation() override;
  bool isPlayingAnimation() override;
  //@}

  //@{
  /**
   * Control if camera movements are enabled, which they are by default
   */
  void enableCameraMovement() override;
  void disableCameraMovement() override;
  //@}

  /**
   * Play a VTK interaction file
   */
  bool playInteraction(const std::string& file) override;

  /**
   * Start interaction and record it all in a VTK interaction file
   */
  bool recordInteraction(const std::string& file) override;

  /**
   * Start the interactor
   */
  void start() override;

  //@{
  /**
   * Implementation only API
   */
  void SetInteractorOn(vtkInteractorObserver* observer);
  void InitializeAnimation(vtkImporter* importer);
  //@}

private:
  class F3DInternals;
  std::unique_ptr<F3DInternals> Internals;
};
}

#endif

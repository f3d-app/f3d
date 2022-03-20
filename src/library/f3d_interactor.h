#ifndef f3d_interactor_h
#define f3d_interactor_h

#include <functional>
#include <memory>
#include <string>
#include <vector>

class vtkInteractorObserver;
class vtkImporter;
namespace f3d
{
class options;
class loader;
class window;
class interactor
{
public:
  interactor(const options& options, window& window, loader& loader);
  ~interactor();

  // XXX is this needed ?
  //  interactor(const interactor& opt);
  //  interactor& operator=(const interactor& opt);

  void setKeyPressCallBack(std::function<bool(int, std::string)> callBack);
  void setDropFilesCallBack(std::function<bool(std::vector<std::string>)> callBack);

  unsigned long createTimerCallBack(double time, std::function<void()> callBack);
  void removeTimerCallBack(unsigned long id);

  //@{
  /**
   * Control the animation
   */
  void toggleAnimation();
  void startAnimation();
  void stopAnimation();
  bool isPlayingAnimation();
  //@}

  //@{
  /**
   * Control if camera movements are enabled, which they are by default
   */
  void enableCameraMovement();
  void disableCameraMovement();
  //@}

  /**
   * Play a VTK interaction file
   */
  bool playInteraction(const std::string& file);

  /**
   * Start interaction and record it all in a VTK interaction file
   */
  bool recordInteraction(const std::string& file);

  void start();

  // PRIVATE API TODO
  void SetInteractorOn(vtkInteractorObserver* observer);
//  void SetLoader(f3d::loader* loader);
  void InitializeAnimation(vtkImporter* importer);

private:
  class F3DInternals;
  std::unique_ptr<F3DInternals> Internals;
};
}

#endif

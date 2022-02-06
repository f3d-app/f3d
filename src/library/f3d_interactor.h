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
class loader;
class interactor
{
public:
  interactor();
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

  /**
   * Control if camera movements are enabled, which they are by default
   */
  void enableCameraMovement();
  void disableCameraMovement();

  // PRIVATE API TODO
  void SetInteractorOn(vtkInteractorObserver* observer);
  void SetLoader(f3d::loader* loader);
  void InitializeAnimation(vtkImporter* importer);

  // TODO Remove
  bool GetDone();
  void Start();

private:
  class F3DInternals;
  std::unique_ptr<F3DInternals> Internals;
};
}

#endif

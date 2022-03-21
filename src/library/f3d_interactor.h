#ifndef f3d_interactor_h
#define f3d_interactor_h

#include <functional>
#include <string>
#include <vector>

// TODO Doc
namespace f3d
{
class options;
class loader;
class window;
class interactor
{
public:
  virtual void setKeyPressCallBack(std::function<bool(int, std::string)> callBack) = 0;
  virtual void setDropFilesCallBack(std::function<bool(std::vector<std::string>)> callBack) = 0;

  virtual unsigned long createTimerCallBack(double time, std::function<void()> callBack) = 0;
  virtual void removeTimerCallBack(unsigned long id) = 0;

  //@{
  /**
   * Control the animation
   */
  virtual void toggleAnimation() = 0;
  virtual void startAnimation() = 0;
  virtual void stopAnimation() = 0;
  virtual bool isPlayingAnimation() = 0;
  //@}

  //@{
  /**
   * Control if camera movements are enabled, which they are by default
   */
  virtual void enableCameraMovement() = 0;
  virtual void disableCameraMovement() = 0;
  //@}

  /**
   * Play a VTK interaction file
   */
  virtual bool playInteraction(const std::string& file) = 0;

  /**
   * Start interaction and record it all in a VTK interaction file
   */
  virtual bool recordInteraction(const std::string& file) = 0;

  /**
   * Start the interactor
   */
  virtual void start() = 0;

protected:
  interactor() = default;
  virtual ~interactor() = default;
  interactor(const interactor& opt) = delete;
  interactor& operator=(const interactor& opt) = delete;
};
}

#endif

/**
 * @class   interactor
 * @brief   Class used to control interaction and animation
 *
 * A class to control interaction with the window as well as animation.
 * It also provide a timer callback if needed and can record/play interaction file.
 */

#ifndef f3d_interactor_h
#define f3d_interactor_h

#include "export.h"

#include <functional>
#include <string>
#include <vector>

namespace f3d
{
class options;
class loader;
class window;
class F3D_EXPORT interactor
{
public:
  /**
   * Use this method to specify your own keypress callback, with the expected API:
   * \code
   * bool callBack(int keyCode, std::string keySym)
   * \endcode
   * keyCode being the pressed key, eg: `C` and keySym the key symbol for key which do not have
   * codes, eg: Left, Right, Up, Down, Space, Enter. Your callBack should return true if the key was
   * handled, false if you want standard interactor behavior instead.
   */
  virtual interactor& setKeyPressCallBack(
    std::function<bool(int, std::string)> callBack) noexcept = 0;

  /**
   * Use this method to specify your own drop files callback, with the expected API:
   * \code
   * bool callBack(std::vector<std::string> files)
   * \endcode
   * files being a vector of string containing paths to dropped files.
   * Your callBack should return true if the event was handled, false if you want standard
   * interactor behavior instead.
   */
  virtual interactor& setDropFilesCallBack(
    std::function<bool(std::vector<std::string>)> callBack) noexcept = 0;

  /**
   * Use this method to create your own timer callback. You callback will be called once every time
   * ms. Return an id to use in removeTimeCallBack
   */
  virtual unsigned long createTimerCallBack(
    double time, std::function<void()> callBack) noexcept = 0;

  /**
   * Remove a previously created timer callback using the id
   */
  virtual void removeTimerCallBack(unsigned long id) noexcept = 0;

  //@{
  /**
   * Control the animation
   */
  virtual void toggleAnimation() noexcept = 0;
  virtual void startAnimation() noexcept = 0;
  virtual void stopAnimation() noexcept = 0;
  virtual bool isPlayingAnimation() noexcept = 0;
  //@}

  //@{
  /**
   * Control if camera movements are enabled, which they are by default
   */
  virtual void enableCameraMovement() noexcept = 0;
  virtual void disableCameraMovement() noexcept = 0;
  //@}

  /**
   * Play a VTK interaction file
   */
  virtual bool playInteraction(const std::string& file) noexcept = 0;

  /**
   * Start interaction and record it all in a VTK interaction file
   */
  virtual bool recordInteraction(const std::string& file) noexcept = 0;

  /**
   * Start the interactor
   */
  virtual void start() noexcept = 0;

  /**
   * Stop the interactor
   */
  virtual void stop() noexcept = 0;

  /**
   * Get a structure of strings describing default interactions
   */
  static const std::vector<std::pair<std::string, std::string> >& getDefaultInteractionsInfo()
    noexcept;

protected:
  interactor() noexcept = default;
  virtual ~interactor() noexcept = default;
  interactor(const interactor& opt) = delete;
  interactor& operator=(const interactor& opt) = delete;
};
}

#endif

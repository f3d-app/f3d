/**
 * @class   window
 * @brief   Abstract class to render in a window or an image
 *
 * A class to render things in a window or an image.
 * An icon and windowName can be set which can be shown by a window manager.
 */

#ifndef f3d_window_h
#define f3d_window_h

#include "export.h"
#include "image.h"

#include <string>

namespace f3d
{
class F3D_EXPORT window
{
public:
  class exception : public std::runtime_error
  {
  public:
    exception(const std::string& what = "")
      : std::runtime_error(what)
    {
    }
  };

  /**
   * Enumeration of supported window types
   * =====================================
   * NONE: A mock window without rendering capabilities
   * NATIVE_WINDOW: A window using the native graphical stack
   * NATIVE_OFFSCREEN: A native window rendering to an offscreen buffer, not visible on screen
   * EXTERNAL_WINDOW: An external window that assume the OpenGL context would have been created by
   * another framework
   */
  enum class Type : unsigned char
  {
    NONE,
    NATIVE,
    NATIVE_OFFSCREEN,
    EXTERNAL
  };

  /**
   * Get the type of the window
   */
  virtual Type getType() = 0;

  /**
   * Perform a render of the window to the screen.
   * All dynamic options are updated if needed.
   * Return true on success, false otherwise.
   */
  virtual bool render() = 0;

  /**
   * Perform a render of the window to the screen and save the result in a f3d::image.
   * Set noBackground to true to have a transparent background.
   * Return the resulting f3d::image.
   */
  virtual image renderToImage(bool noBackground = false) = 0;

  /**
   * Set the size of the window
   */
  virtual void setSize(int width, int heigth) = 0;

  /**
   * Set the icon to be shown by a window manager.
   * icon should be an unsigned char array
   * iconSize should be the sizeof(icon)
   * Return true on success, false otherwise.
   */
  virtual bool setIcon(const void* icon, size_t iconSize) = 0;

  /**
   * Set the window name to be shown by a window manager.
   * Return true on success, false otherwise.
   */
  virtual bool setWindowName(const std::string& windowName) = 0;

protected:
  window() = default;
  virtual ~window() = default;
  window(const window&) = delete;
  window& operator=(const window&) = delete;
};
}

#endif

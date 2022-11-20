#ifndef f3d_window_h
#define f3d_window_h

#include "camera.h"
#include "export.h"
#include "image.h"

#include <string>

namespace f3d
{
/**
 * @class   window
 * @brief   Abstract class to render in a window or an image
 *
 * A class to render things in a window or an image.
 * An icon and windowName can be set which can be shown by a window manager.
 */
class F3D_EXPORT window
{
public:
  /**
   * Enumeration of supported window types
   * =====================================
   * NONE: A mock window without rendering capabilities.
   * NATIVE: A window using the native graphical stack.
   * NATIVE_OFFSCREEN: A native window rendering to an offscreen buffer, not visible on screen.
   * EXTERNAL: An external window that assume the OpenGL context would have been created by.
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
   * Get the type of the window.
   */
  virtual Type getType() = 0;

  /**
   * Get the camera provided by the window.
   */
  virtual camera& getCamera() = 0;

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
   * Set the size of the window.
   */
  virtual window& setSize(int width, int height) = 0;

  /**
   * Get the width of the window.
   */
  virtual int getWidth() const = 0;

  /**
   * Get the height of the window.
   */
  virtual int getHeight() const = 0;

  /**
   * Set the position of the window.
   */
  virtual window& setPosition(int x, int y) = 0;

  /**
   * Set the icon to be shown by a window manager.
   * icon should be an unsigned char array.
   * iconSize should be the sizeof(icon).
   */
  virtual window& setIcon(const unsigned char* icon, size_t iconSize) = 0;

  /**
   * Set the window name to be shown by a window manager.
   */
  virtual window& setWindowName(const std::string& windowName) = 0;

  /**
   * Convert a point in display coordinate to world coordinate.
   */
  virtual point3_t getWorldFromDisplay(const point3_t& displayPoint) const = 0;

  /**
   * Convert a point in world coordinate to display coordinate.
   */
  virtual point3_t getDisplayFromWorld(const point3_t& worldPoint) const = 0;

protected:
  //! @cond
  window() = default;
  virtual ~window() = default;
  window(const window&) = delete;
  window(window&&) = delete;
  window& operator=(const window&) = delete;
  window& operator=(window&&) = delete;
  //! @endcond
};
}

#endif

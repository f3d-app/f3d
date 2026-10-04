#ifndef f3d_animation_h
#define f3d_animation_h

#include "export.h"
#include "options.h"
#include "window.h"

namespace f3d
{
/**
 * @class   animation
 * @brief   abstract to handle animation control in the libf3d
 *
 * A class to control everything related to animation
 * in the libf3d. It recovers information from the currently loaded
 * files in the scene.
 */
class F3D_EXPORT animation
{
public:
  /**
   * Load files in the scene at provided time value if they contain any animation
   * Providing a time value outside of the current animation time range will clamp
   * to the closest value in the range.
   * Does not do anything if there is no animations.
   */
  virtual animation& loadTime(double timeValue) = 0;

  /**
   * Get animation time range of currently added files in the scene.
   * Returns [0, 0] if there is no animations.
   */
  [[nodiscard]] virtual std::pair<double, double> timeRange() = 0;

  /**
   * Get animation keyframe's time of currently added files.
   * Can be used in loadTime to request a specific keyframe.
   * Returns empty vector if there is no animations.
   */
  [[nodiscard]] virtual std::vector<double> keyFrames() = 0;

  /**
   * Return the number of animations available in the currently loaded files.
   */
  [[nodiscard]] virtual unsigned int count() const = 0;

  /**
   * Return the animation name of a given animation index, if any.
   *
   * Specific animation (0..count): Returns the name of the animation at that index
   * Current animation (-1):
   *   - Returns the name of the current animation
   *   - Returns "Multi animations" if more than one animation is current
   *   - Returns "All animations" if all animations are current
   *   - Returns "No animations" if no animations are current
   * Fallback: Returns "No animation" for out-of-bounds requests.
   */
  [[nodiscard]] virtual std::string getName(int index = -1) const = 0;

  /**
   * Return all of the animation names, if any.
   * Returns a vector of length 0 if none.
   */
  [[nodiscard]] virtual std::vector<std::string> getNames() const = 0;

protected:
  //! @cond
  animation() = default;
  virtual ~animation() = default;
  animation(const animation& opt) = delete;
  animation(animation&& opt) = delete;
  animation& operator=(const animation& opt) = delete;
  animation& operator=(animation&& opt) = delete;
  //! @endcond
};
}

#endif

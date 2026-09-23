#ifndef f3d_animation_h
#define f3d_animation_h

#include "export.h"
#include "options.h"
#include "window.h"

namespace f3d
{
/**
 * @class   animation
 * @brief   TODO
 *
 * TODO
 */
class F3D_EXPORT animation
{
public:
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

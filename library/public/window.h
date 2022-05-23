#ifndef f3d_window_h
#define f3d_window_h

#include "export.h"
#include "image.h"

#include <string>

// TODO DOC
namespace f3d
{
class F3D_EXPORT window
{
public:
  virtual bool update() = 0;
  virtual bool render() = 0;
  virtual image renderToImage(bool noBackground = false) = 0;
  virtual bool setIcon(const void* icon, size_t iconSize) = 0;
  virtual bool setWindowName(const std::string& windowName) = 0;

protected:
  window() = default;
  virtual ~window() = default;
  window(const window&) = delete;
  window& operator=(const window&) = delete;
};
}

#endif

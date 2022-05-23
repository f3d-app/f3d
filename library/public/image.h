/**
 * @class   image
 * @brief   Class used to represent an image
 *
 * TODO improve doc
 */

#ifndef f3d_image_h
#define f3d_image_h

#include "export.h"

#include <string>

namespace f3d
{
class F3D_EXPORT image
{
public:
  image();
  explicit image(const std::string& path);

  ~image();

  image(const image& img);
  image& operator=(const image& img);

  int getWidth() const;
  int getHeight() const;

  image& setResolution(int width, int height);

  int getChannelCount() const;
  image& setChannelCount(int dim);

  image& setData(unsigned char* buffer);
  unsigned char* getData() const;

  bool compare(const image& reference, image& result, double threshold) const;

  image& save(const std::string& path);

private:
  class internals;
  internals* Internals;
};
}

#endif

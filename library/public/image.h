/**
 * @class   image
 * @brief   Class used to represent an image
 *
 * TODO improve doc
 */

#ifndef f3d_image_h
#define f3d_image_h

#include "export.h"

#include <stdexcept>
#include <string>

namespace f3d
{
class F3D_EXPORT image
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

  image();
  explicit image(const std::string& path);

  ~image();

  image(const image& img);
  image& operator=(const image& img);

  image(image&& img);
  image& operator=(image&& img);

  unsigned int getWidth() const;
  unsigned int getHeight() const;

  image& setResolution(unsigned int width, unsigned int height);

  unsigned int getChannelCount() const;
  image& setChannelCount(unsigned int dim);

  image& setData(unsigned char* buffer);
  unsigned char* getData() const;

  bool compare(const image& reference, image& result, double threshold, double& error) const;

  bool operator==(const image& reference) const;
  bool operator!=(const image& reference) const;

  image& save(const std::string& path);

private:
  class internals;
  internals* Internals;
};
}

#endif

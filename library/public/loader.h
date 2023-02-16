#ifndef f3d_loader_h
#define f3d_loader_h

#include "export.h"

#include <string>
#include <vector>

namespace f3d
{
/**
 * @class   loader
 * @brief   Class to load files in F3D
 *
 * A class to load files in F3D.
 * TODO
 */
class F3D_EXPORT loader
{
public:
  /**
   * Load the provided file and associated fileInfo TODO improve
   * Returns true if a file is loaded successfully, false otherwise.
   */
  virtual bool loadFile(const std::string& filePath, const std::string& fileInfo = "") = 0;

protected:
  //! @cond
  loader() = default;
  virtual ~loader() = default;
  loader(const loader& opt) = delete;
  loader(loader&& opt) = delete;
  loader& operator=(const loader& opt) = delete;
  loader& operator=(loader&& opt) = delete;
  //! @endcond
};
}

#endif

#ifndef f3d_loader_h
#define f3d_loader_h

#include "exception.h"
#include "export.h"
#include "types.h"

#include <filesystem>
#include <string>
#include <vector>

namespace f3d
{
/**
 * @class   loader
 * @brief   Class to load files in F3D
 *
 * A class to load files in F3D. The loader can load a full scene or multiple geometries into a
 * default scene. It also support checking if a scene or geometry reader is available for a given
 * file.
 *
 * Example usage:
 * \code{.cpp}
 *  std::string path = ...
 *  f3d::engine eng(f3d::window::Type::NATIVE);
 *  f3d::loader& load = eng.getLoader();
 *
 *  if (load.supported(path)
 *  {
 *    load.add(path);
 *  }
 * \endcode
 *
 */
class F3D_EXPORT loader
{
public:
  /**
   * An exception that can be thrown by the loader
   * when it failed to load a file for some reason.
   */
  struct load_failure_exception : public exception
  {
    explicit load_failure_exception(const std::string& what = "")
      : exception(what){};
  };

  ///@{
  /**
   * Add and load provided files into the scene
   * Already added file will NOT be reloaded
   */
  virtual loader& add(const std::filesystem::path& filePath) = 0;
  virtual loader& add(const std::vector<std::filesystem::path>& filePath) = 0;
  virtual loader& add(const std::vector<std::string>& filePathStrings) = 0;
  ///@}

  /**
   * Add and load provided mesh into the scene
   */
  virtual loader& add(const mesh_t& mesh) = 0;

  ///@{
  /**
   * Convenience initializer list signature for add method
   */
  loader& add(std::initializer_list<std::string> list)
  {
    return this->add(std::vector<std::string>(list));
  }
  loader& add(std::initializer_list<std::filesystem::path> list)
  {
    return this->add(std::vector<std::filesystem::path>(list));
  }
  ///@}

  /**
   * Clear the scene of all added files
   */
  virtual loader& clear() = 0;

  /**
   * Return true if provided file path is supported, false otherwise.
   */
  virtual bool supported(const std::filesystem::path& filePath) = 0;

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

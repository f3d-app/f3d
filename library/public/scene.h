#ifndef f3d_scene_h
#define f3d_scene_h

#include "exception.h"
#include "export.h"
#include "types.h"

#include <filesystem>
#include <string>
#include <vector>

namespace f3d
{
/**
 * @class   scene
 * @brief   Class to load files into
 *
 * The scene where files and meshes can be added and loaded into.
 *
 * Example usage:
 * \code{.cpp}
 *  std::string path = ...
 *  f3d::engine eng(f3d::window::Type::NATIVE);
 *  f3d::scene& load = eng.getScene();
 *
 *  if (load.supports(path)
 *  {
 *    load.add(path);
 *  }
 * \endcode
 *
 */
class F3D_EXPORT scene
{
public:
  /**
   * An exception that can be thrown by the scene
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
   * If it fails to loads a file, it clear the scene and
   * throw a load_failure_exception.
   */
  virtual scene& add(const std::filesystem::path& filePath) = 0;
  virtual scene& add(const std::vector<std::filesystem::path>& filePath) = 0;
  virtual scene& add(const std::vector<std::string>& filePathStrings) = 0;
  ///@}

  /**
   * Add and load provided mesh into the scene
   */
  virtual scene& add(const mesh_t& mesh) = 0;

  ///@{
  /**
   * Convenience initializer list signature for add method
   */
  scene& add(std::initializer_list<std::string> list)
  {
    return this->add(std::vector<std::string>(list));
  }
  scene& add(std::initializer_list<std::filesystem::path> list)
  {
    return this->add(std::vector<std::filesystem::path>(list));
  }
  ///@}

  /**
   * Clear the scene of all added files
   */
  virtual scene& clear() = 0;

  /**
   * Return true if provided file path is supported, false otherwise.
   */
  [[nodiscard]] virtual bool supports(const std::filesystem::path& filePath) = 0;

  /**
   * Load added files at provided time value if they contain any animation
   * Providing a timeVale outside of the current animationTimeRange will clamp
   * to the closest value in the range.
   * Does not do anything if there is no animations.
   */
  virtual scene& loadAnimationTime(double timeValue) = 0;

  /**
   * Get animation time range of currently added files.
   * Returns [0, 0] if there is no animations.
   */
  [[nodiscard]] virtual std::pair<double, double> animationTimeRange() = 0;

protected:
  //! @cond
  scene() = default;
  virtual ~scene() = default;
  scene(const scene& opt) = delete;
  scene(scene&& opt) = delete;
  scene& operator=(const scene& opt) = delete;
  scene& operator=(scene&& opt) = delete;
  //! @endcond
};
}

#endif

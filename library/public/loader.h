#ifndef f3d_loader_h
#define f3d_loader_h

#include "exception.h"
#include "export.h"

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
 *  if (load.hasSceneReader(path)
 *  {
 *    load.loadScene(path);
 *  }
 *  else if (load.hasGeometryReader(path)
 *  {
 *    load.loadGeometry(path);
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

  /**
   * Return true if the loader has a geometry reader for the providen file, false otherwise.
   */
  virtual bool hasGeometryReader(const std::string& filePath) = 0;

  /**
   * Load a geometry from a provided file to the scene.
   * Reset the scene before loading if a full scene was loaded previously or if reset is set to
   * false, do not reset if only loaded geometries previously. Geometries loader using this method
   * will be available in a default scene and use all default scene related options. Throw a
   * load_failure_exception on failure.
   */
  virtual loader& loadGeometry(const std::string& filePath, bool reset = false) = 0;

  /**
   * Return true if the loader has a scene reader for the providen file, false otherwise.
   */
  virtual bool hasSceneReader(const std::string& filePath) = 0;

  /**
   * Reset scene and load a (full) scene from provided file.
   * Please note default scene related options are not taken into account when loading a full scene.
   * Throw a load_failure_exception on failure.
   */
  virtual loader& loadScene(const std::string& filePath) = 0;

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

/**
 * @class   F3DStarter
 * @brief   The starter class
 *
 */

#ifndef F3DStarter_h
#define F3DStarter_h

// TODO: this file is used in the application so it needs to be exported.
// However, it will be moved completely in the application at some point, then export of used APIs
// can be removed
#include "f3d_export.h"

#include "f3d_loader.h"

class F3DStarter
{
public:
  /**
   * Parse the options and configure a f3d::loader accordingly
   */
  F3D_EXPORT int Start(int argc, char** argv);

  /**
   * Add a file or directory to be forwarded to the loader
   */
  void AddFile(const std::string& path);

  /**
   * Load a file if any have been added
   * Set the load argument to LOAD_FIRST, LOAD_PREVIOUS, LOAD_NEXT or LOAD_LAST to change file index
   * Returns true if file is loaded sucessfully, false otherwise
   */
  bool LoadFile(f3d::loader::LoadFileEnum load = f3d::loader::LoadFileEnum::LOAD_CURRENT);

  F3D_EXPORT F3DStarter();
  F3D_EXPORT ~F3DStarter();

private:
  class F3DInternals;
  std::unique_ptr<F3DInternals> Internals;

  F3DStarter(F3DStarter const&) = delete;
  void operator=(F3DStarter const&) = delete;
};

#endif

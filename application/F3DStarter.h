/**
 * @class   F3DStarter
 * @brief   The starter class
 *
 */

#ifndef F3DStarter_h
#define F3DStarter_h

#include "loader.h"

#include <memory>

class F3DStarter
{
public:
  /**
   * Parse the options and configure a f3d::loader accordingly
   */
  int Start(int argc, char** argv);

  /**
   * Add a file or directory to be forwarded to the loader
   */
  void AddFile(const std::string& path);

  /**
   * Load a file if any have been added
   * Set the load argument to LOAD_FIRST, LOAD_PREVIOUS, LOAD_NEXT or LOAD_LAST to change file index
   */
  void LoadFile(f3d::loader::LoadFileEnum load = f3d::loader::LoadFileEnum::LOAD_CURRENT);

  /**
   * Trigger a render
   */
  void Render();

  F3DStarter();
  ~F3DStarter();

private:
  class F3DInternals;
  std::unique_ptr<F3DInternals> Internals;

  F3DStarter(F3DStarter const&) = delete;
  void operator=(F3DStarter const&) = delete;
};

#endif

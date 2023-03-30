/**
 * @class   F3DStarter
 * @brief   The starter class
 *
 */

#ifndef F3DStarter_h
#define F3DStarter_h

#include "loader.h"

#include <filesystem>
#include <memory>

class F3DStarter
{
public:
  /**
   * Parse the options and configure a f3d::loader accordingly
   */
  int Start(int argc, char** argv);

  /**
   * Add a file or directory to the list of paths
   */
  void AddFile(const std::filesystem::path& path);

  /**
   * Load a file if any have been added
   * Set the index to select the index of the file to load
   * Set relativeIndex to true to use the index as a relative index with the current index
   */
  void LoadFile(int index = 0, bool relativeIndex = false);

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

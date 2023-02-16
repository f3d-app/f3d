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
   * An enum used in loadFile to select which file to load.
   */
  enum LoadFileEnum : int
  {
    LOAD_PREVIOUS = -1,
    LOAD_CURRENT = 0,
    LOAD_NEXT = 1,
    LOAD_LAST = 2,
  };

  /**
   * Parse the options and configure a f3d::loader accordingly
   */
  int Start(int argc, char** argv);

  /**
   * Add a file or directory to be forwarded to the loader
   */
  void AddFile(const std::string& path);

  /**
   * Get information about the next file to load according to the load param.
   * Set the load argument to LOAD_FIRST, LOAD_PREVIOUS, LOAD_NEXT or LOAD_LAST as needed.
   * nextFileIndex will provide the index of the file or -1 if not available.
   * filePath the path to the file and fileInfo a more complete information about the file,
   * including index in list.
   * TODO rework this method
   */
  //  virtual void getFileInfo(LoadFileEnum load, int& nextFileIndex, std::string& filePath,
  //    std::string& fileName, std::string& fileInfo) const = 0;

  /**
   * Load a file if any have been added
   * Set the load argument to LOAD_FIRST, LOAD_PREVIOUS, LOAD_NEXT or LOAD_LAST to change file index
   */
  void LoadFile(LoadFileEnum load = LoadFileEnum::LOAD_CURRENT);

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

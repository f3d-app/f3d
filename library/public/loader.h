/**
 * @class   loader
 * @brief   Class to load files in F3D
 *
 * A class to load files in F3D.
 * Multiple files and folders can be added using addFiles/addFile and stored
 * in an internal list, with a index pointing to a "current" file.
 * Then files from the list can be loaded one by one using loadFile method.
 * Information about the files in the list can be recovered using getFileInfo.
 * The current file can be changed either on loadFile with a dedicated enum or
 * by using setCurrentFileIndex.
 */

#ifndef f3d_loader_h
#define f3d_loader_h

#include "export.h"

#include <string>
#include <vector>

namespace f3d
{
class F3D_EXPORT loader
{
public:
  /**
   * An enum used in loadFile to select which file to load.
   */
  enum class LoadFileEnum
  {
    LOAD_FIRST,
    LOAD_PREVIOUS,
    LOAD_CURRENT,
    LOAD_NEXT,
    LOAD_LAST
  };

  /**
   * Add a list of files or directories to be loaded.
   * Duplicate will not be added.
   */
  virtual void addFiles(const std::vector<std::string>& files) noexcept = 0;

  /**
   * Add a file or directory (all files in) to be loaded.
   * Set recursive to true to load folder recursively instead of only files.
   */
  virtual void addFile(const std::string& path, bool recursive = false) noexcept = 0;

  /**
   * Get the vector of files to be loaded.
   */
  virtual const std::vector<std::string>& getFiles() const noexcept = 0;

  /**
   * Set the current file index, corresponding to the file that will be loaded
   * when calling loadFile(LOAD_CURRENT).
   */
  virtual void setCurrentFileIndex(int index) noexcept = 0;

  /**
   * Get the current file index.
   */
  virtual int getCurrentFileIndex() const noexcept = 0;

  /**
   * Load a file if any have been added.
   * Set the load argument to LOAD_FIRST, LOAD_PREVIOUS, LOAD_NEXT or LOAD_LAST to change file
   * index. This will change the current file index accordingly. Returns true if a file is loaded
   * sucessfully, false otherwise.
   */
  virtual bool loadFile(LoadFileEnum load) noexcept = 0;

  /**
   * Get information about the next file to load according to the load param.
   * Set the load argument to LOAD_FIRST, LOAD_PREVIOUS, LOAD_NEXT or LOAD_LAST as needed.
   * nextFileIndex will provide the index of the file or -1 if not available.
   * filePath the path to the file and fileInfo a more complete information about the file,
   * including index in list.
   */
  virtual void getFileInfo(LoadFileEnum load, int& nextFileIndex, std::string& filePath,
    std::string& fileInfo) const noexcept = 0;

protected:
  loader() noexcept = default;
  virtual ~loader() noexcept = default;
  loader(const loader& opt) = delete;
  loader& operator=(const loader& opt) = delete;
};
}

#endif

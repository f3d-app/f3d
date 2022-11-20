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
 * Multiple files and folders can be added using addFiles/addFile and stored
 * in an internal list, with a index pointing to a "current" file.
 * Then files from the list can be loaded one by one using loadFile method.
 * Information about the files in the list can be recovered using getFileInfo.
 * The current file can be changed either on loadFile with a dedicated enum or
 * by using setCurrentFileIndex.
 */
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
   * Set recursive to true to load folder recursively instead of only files.
   * Duplicate will not be added.
   */
  virtual loader& addFiles(const std::vector<std::string>& files, bool recursive = false) = 0;

  /**
   * Add a file or directory (all files in) to be loaded.
   * Set recursive to true to load folder recursively instead of only files.
   */
  virtual loader& addFile(const std::string& path, bool recursive = false) = 0;

  /**
   * Get the vector of files to be loaded.
   */
  virtual const std::vector<std::string>& getFiles() const = 0;

  /**
   * Set the current file index, corresponding to the file that will be loaded
   * when calling loadFile(LOAD_CURRENT).
   */
  virtual loader& setCurrentFileIndex(int index) = 0;

  /**
   * Get the current file index.
   */
  virtual int getCurrentFileIndex() const = 0;

  /**
   * Load a file if any have been added.
   * Set the load argument to LOAD_FIRST, LOAD_PREVIOUS, LOAD_NEXT or LOAD_LAST to change file
   * index. This will change the current file index accordingly.
   * Default is LOAD_CURRENT.
   * Returns true if a file is loaded successfully, false otherwise.
   */
  virtual bool loadFile(LoadFileEnum load = LoadFileEnum::LOAD_CURRENT) = 0;

  /**
   * Get information about the next file to load according to the load param.
   * Set the load argument to LOAD_FIRST, LOAD_PREVIOUS, LOAD_NEXT or LOAD_LAST as needed.
   * nextFileIndex will provide the index of the file or -1 if not available.
   * filePath the path to the file and fileInfo a more complete information about the file,
   * including index in list.
   */
  virtual void getFileInfo(
    LoadFileEnum load, int& nextFileIndex, std::string& filePath, std::string& fileInfo) const = 0;

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

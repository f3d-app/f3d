#ifndef f3d_loader_h
#define f3d_loader_h

#include <memory>
#include <string>
#include <vector>

// TODO doc
namespace f3d
{
class loader
{
public:
  enum class LoadFileEnum
  {
    LOAD_FIRST,
    LOAD_PREVIOUS,
    LOAD_CURRENT,
    LOAD_NEXT,
    LOAD_LAST
  };

  /**
   * Add a list of files or directory to be loaded
   */
  virtual void addFiles(const std::vector<std::string>& files) = 0;

  /**
   * Add a file or directory to be loaded
   * Set recursive to true to add all the files in a directory
   */
  virtual void addFile(const std::string& path, bool recursive = true) = 0;

  /**
   * Get the vector of files to be loaded
   */
  virtual std::vector<std::string> getFiles() = 0;

  /**
   * Set the current file index
   */
  virtual void setCurrentFileIndex(int index) = 0;

  /**
   * Get the current file index
   */
  virtual int getCurrentFileIndex() = 0;

  /**
   * Load a file if any have been added
   * Set the load argument to LOAD_FIRST, LOAD_PREVIOUS, LOAD_NEXT or LOAD_LAST to change file index
   * Returns true if a file is loaded sucessfully, false otherwise
   */
  virtual bool loadFile(LoadFileEnum load) = 0;

  /**
   * Get information about the next file to load according to the load param
   * Set the load argument to LOAD_FIRST, LOAD_PREVIOUS, LOAD_NEXT or LOAD_LAST as needed
   * nextFileIndex will provide the index of the file or -1 if not available
   * filePath the path to the file and fileInfo a more complete information about the file
   * Return true if there is a file or false otherwise
   */
  virtual void getFileInfo(
    LoadFileEnum load, int& nextFileIndex, std::string& filePath, std::string& fileInfo) const = 0;

protected:
  loader() = default;
  virtual ~loader() = default;
  loader(const loader& opt) = delete;
  loader& operator=(const loader& opt) = delete;
};
}

#endif

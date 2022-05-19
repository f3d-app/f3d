#ifndef f3d_loader_impl_h
#define f3d_loader_impl_h

#include "f3d_loader.h"

namespace f3d
{
class options;

namespace detail
{
class interactor_impl;
class window_impl;
class loader_impl : public loader
{
public:
  loader_impl(const options& options, window_impl& window);
  ~loader_impl();

  /**
   * Add a list of files or directory to be loaded
   */
  void addFiles(const std::vector<std::string>& files) override;

  /**
   * Add a file or directory to be loaded
   * Set recursive to true to add all the files in a directory
   */
  void addFile(const std::string& path, bool recursive = true) override;

  /**
   * Get the vector of files to be loaded
   */
  std::vector<std::string> getFiles() override;

  /**
   * Set the current file index
   */
  void setCurrentFileIndex(int index) override;

  /**
   * Get the current file index
   */
  int getCurrentFileIndex() override;

  /**
   * Load a file if any have been added
   * Set the load argument to LOAD_FIRST, LOAD_PREVIOUS, LOAD_NEXT or LOAD_LAST to change file index
   * Returns true if a file is loaded sucessfully, false otherwise
   */
  bool loadFile(LoadFileEnum load) override;

  /**
   * Get information about the next file to load according to the load param
   * Set the load argument to LOAD_FIRST, LOAD_PREVIOUS, LOAD_NEXT or LOAD_LAST as needed
   * nextFileIndex will provide the index of the file or -1 if not available
   * filePath the path to the file and fileInfo a more complete information about the file
   * Return true if there is a file or false otherwise
   */
  void getFileInfo(LoadFileEnum load, int& nextFileIndex, std::string& filePath,
    std::string& fileInfo) const override;

  /**
   * Implementation only API.
   * Set the interactor to use when interacting
   */
  void setInteractor(interactor_impl* interactor);

private:
  class internals;
  std::unique_ptr<internals> Internals;
};
}
}

#endif

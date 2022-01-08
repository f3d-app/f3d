/**
 * @class   F3DLoader
 * @brief   The loader class
 *
 */

#ifndef F3DLoader_h
#define F3DLoader_h

#include <memory>
#include <string>
#include <vector>

class F3DLoaderInternals;
class vtkImageData;

class F3DLoader
{
public:
  enum LoadFileEnum
  {
    LOAD_PREVIOUS = -1,
    LOAD_CURRENT = 0,
    LOAD_NEXT = 1
  };

  /**
   * Parse the options, create an importer and start the rendering
   * This will initialize the following members:
   * Parser, CommandLineOptions, AnimationManager, RenWin
   */
  int Start(int argc, char** argv, vtkImageData* image = nullptr);

  /**
   * Add a list of files or directory to be loaded
   * to the FilesList
   */
  void AddFiles(const std::vector<std::string>& files);

  /**
   * Add a file or directory to be loaded to the FilesList
   * Set recursive to true to add all the file in a directory
   */
  void AddFile(const std::string& path, bool recursive = true);

  /**
   * Load a file if any have been added
   * Set the load argument to LOAD_PREVIOUS or LOAD_NEXT to change file index
   * This will initialize the following members:
   * Options, Renderer, Importer
   * Returns true if file is loaded sucessfully, false otherwise
   */
  bool LoadFile(int load = F3DLoader::LOAD_CURRENT);

  F3DLoader();
  ~F3DLoader();

private:
  std::unique_ptr<F3DLoaderInternals> Internals;

  F3DLoader(F3DLoader const&) = delete;
  void operator=(F3DLoader const&) = delete;
};

#endif

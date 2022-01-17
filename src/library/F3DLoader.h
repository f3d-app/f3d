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

namespace f3d
{
class options;
};
class vtkF3DInteractorStyle;

class F3DLoader
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
  bool LoadFile(LoadFileEnum load, const f3d::options& options);

  /**
   * Forward to AnimationManager
   */
  void ToggleAnimation();

  /**
   * Initialize the rendering stack managed by the loader
   * This will initialize the following members:
   * AnimationManager, RenWin, Interactor, Style
   */
  void InitializeRendering(std::string, bool offscreen, const void* iconBuffer, size_t inconLength);

  /**
   * Convenience method to get the interactor style
   * TODO Remove
   */
  vtkF3DInteractorStyle* GetInteractorStyle();

  /**
   * Get information about the next file to load according to the load param
   * Set the load argument to LOAD_FIRST, LOAD_PREVIOUS, LOAD_NEXT or LOAD_LAST as needed
   * nextFileIndex will provide the index of the file or -1 if not available
   * filePath the path to the file and fileInfo a more complete information about the file
   * Return true if there is a file or false otherwise
   */
  void GetNextFile(
    LoadFileEnum load, int& nextFileIndex, std::string& filePath, std::string& fileInfo) const;

  /**
   * Start the interaction/rendering
   * Return true if rendering and interaction is successful, false otherwise
   */
  bool Start();

  F3DLoader();
  ~F3DLoader();

protected:
  void InitializeRenderer();

private:
  class F3DInternals;
  std::unique_ptr<F3DInternals> Internals;

  F3DLoader(F3DLoader const&) = delete;
  void operator=(F3DLoader const&) = delete;
};

#endif

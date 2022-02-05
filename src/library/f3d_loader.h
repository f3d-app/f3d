#ifndef f3d_loader_h
#define f3d_loader_h

#include <memory>
#include <string>
#include <vector>

class F3DAnimationManager;
class vtkRenderWindow;

namespace f3d
{
class options;
class interactor;
class window;
class loader
{
public:
  loader(const options& options);
  ~loader();

  // XXX is this needed ?
  //  loader(const loader& opt);
  //  loader& operator=(const loader& opt);

  enum class LoadFileEnum
  {
    LOAD_FIRST,
    LOAD_PREVIOUS,
    LOAD_CURRENT,
    LOAD_NEXT,
    LOAD_LAST
  };

  //@{
  /**
   * Get the options used by the loader
   */
  const options& getOptions();
  //@}

  /**
   * Set the interactor to use when interacting
   */
  void setInteractor(interactor* interactor);

  /**
   * Set/get the window to use when rendering
   */
  window* getWindow();
  void setWindow(window* interactor);

  /**
   * Add a list of files or directory to be loaded
   */
  void addFiles(const std::vector<std::string>& files);

  /**
   * Add a file or directory to be loaded
   * Set recursive to true to add all the files in a directory
   */
  void addFile(const std::string& path, bool recursive = true);

  /**
   * Get the vector of files to be loaded
   */
  std::vector<std::string> getFiles();

  /**
   * Set the current file index
   */
  void setCurrentFileIndex(int index);

  /**
   * Get the current file index
   */
  int getCurrentFileIndex();

  /**
   * Load a file if any have been added
   * Set the load argument to LOAD_FIRST, LOAD_PREVIOUS, LOAD_NEXT or LOAD_LAST to change file index
   * Returns true if a file is loaded sucessfully, false otherwise
   */
  bool loadFile(LoadFileEnum load);

  /**
   * Get information about the next file to load according to the load param
   * Set the load argument to LOAD_FIRST, LOAD_PREVIOUS, LOAD_NEXT or LOAD_LAST as needed
   * nextFileIndex will provide the index of the file or -1 if not available
   * filePath the path to the file and fileInfo a more complete information about the file
   * Return true if there is a file or false otherwise
   */
  void getFileInfo(
    LoadFileEnum load, int& nextFileIndex, std::string& filePath, std::string& fileInfo) const;

  /**
   * Trigger a rendering
   * TODO Move that to another class
   */
  void render();

  /**
   * Start the interaction/rendering
   * Return true if rendering and interaction is successful, false otherwise
   * TODO Move that to another class
   */
  bool start();

  /**
   * Forward to AnimationManager to start the animation
   * TODO Move that to another class
   */
  void toggleAnimation();

  /**
   * Initialize the rendering stack managed by the loader
   * This will initialize the following members:
   * AnimationManager, RenWin
   * TODO Manage that internally depending on the type of context provided
   */
  //  void InitializeRendering(std::string, bool offscreen, const void* iconBuffer, size_t
  //  inconLength);

  // TODO PRIVATE API
  const F3DAnimationManager* GetAnimationManager();

private:
  class F3DInternals;
  std::unique_ptr<F3DInternals> Internals;
};
}

#endif

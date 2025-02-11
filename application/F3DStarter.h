/**
 * @class   F3DStarter
 * @brief   The starter class
 *
 */

#ifndef F3DStarter_h
#define F3DStarter_h

#include <filesystem>
#include <memory>
#include <vector>

class F3DStarter
{
public:
  /**
   * Parse the options and configure a f3d::scene accordingly
   */
  int Start(int argc, char** argv);

  /**
   * Add a file or directory to a file group
   * Returns the index of the group where the file was added
   */
  int AddFile(const std::filesystem::path& path, bool quiet = false);

  /**
   * Load a file group if any have been added
   * Set the index to select the index of the file group to load
   * Set relativeIndex to true to use the index as a relative index with the current index
   */
  void LoadFileGroup(int index = 0, bool relativeIndex = false, bool forceClear = false);

  /**
   * Trigger a render on the next event loop
   */
  void RequestRender();

  /**
   * Trigger a render immediately (must be called by the main thread)
   */
  void Render();

  /**
   * Trigger a render and save a screenshot to disk according to a filename template.
   * See `F3DStarter::F3DInternals::applyFilenameTemplate` for template substitution details.
   * If the `minimal` parameter is `true`, render with transparent background, no grid,
   * and no overlays.
   */
  void SaveScreenshot(const std::string& filenameTemplate, bool minimal = false);

  F3DStarter();
  ~F3DStarter();

  F3DStarter(F3DStarter const&) = delete;
  void operator=(F3DStarter const&) = delete;

private:
  class F3DInternals;
  std::unique_ptr<F3DInternals> Internals;

  /**
   * Internal method triggered when interacting with the application
   * that load a file group using relative index and handle camera restore
   */
  bool LoadRelativeFileGroup(
    int relativeIndex = 0, bool restoreCamera = false, bool forceClear = false);

  /**
   * Internal method used to load a provided file group into the scene.
   * Set clear to true to clear the scene first
   * GroupIdx is only used for display purposes of the filename
   */
  void LoadFileGroup(
    const std::vector<std::filesystem::path>& paths, bool clear, const std::string& groupIdx);

  /**
   * Internal event loop that is triggered repeatedly to handle specific events:
   * - Render
   * - ReloadFile
   */
  void EventLoop();

  /**
   * Add F3D specific commands
   */
  void AddCommands();
};

#endif

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
   * Returns the index of the added file
   */
  int AddFile(const std::filesystem::path& path, bool quiet = false);

  /**
   * Load a file if any have been added
   * Set the index to select the index of the file to load
   * Set relativeIndex to true to use the index as a relative index with the current index
   */
  void LoadFile(int index = 0, bool relativeIndex = false);

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
   * that load a file using relative index and handle camera restore
   */
  bool LoadRelativeFile(int relativeIndex = 0, bool restoreCamera = false);

  /**
   * Internal event loop that is triggered repeatedly to handle specific events:
   * - Render
   * - ReloadFile
   */
  void EventLoop();
};

#endif

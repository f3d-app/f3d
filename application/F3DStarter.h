/**
 * @class   F3DStarter
 * @brief   The starter class
 *
 */

#ifndef F3DStarter_h
#define F3DStarter_h

#include <filesystem>
#include <map>
#include <memory>
#include <optional>
#include <string>
#include <utility>
#include <vector>

class F3DStarter
{
public:
  /**
   * App-specific file group data stored in a statefile, alongside the libf3d state.
   * Each group is a (key, files) pair, matching F3DInternals::FilesGroups, and Current is the
   * index of the group that was loaded when the statefile was saved.
   */
  struct StatefileFileGroups
  {
    std::vector<std::pair<std::string, std::vector<std::filesystem::path>>> Groups;
    int Current = 0;
  };

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
   * Get supported extensions for open file command
   * On macOS, the extensions are returned in the format: {"x", "y", "z"}
   * On other operating systems, the format is: {"*.x", "*.y", "*.z"}
   * Returns a vector of supported file extension strings
   */
  static std::vector<std::string> GetExtensions();

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

  /**
   * Save the current engine state to a statefile.
   * `filenameTemplate` supports the same template variables as the output.
   * If it is `-`, the statefile is written to the standard output.
   */
  void SaveStatefile(const std::string& filenameTemplate);

  /**
   * Save the current engine state to the system clipboard.
   * Requires a build with the clip module, logs an error otherwise.
   */
  void SaveStatefileToClipboard();

  /**
   * Restore the engine state from a statefile, reloading the saved files in the process.
   * `source` is a file path or `-` to read from the standard input.
   */
  void LoadStatefile(const std::string& source);

  /**
   * Restore the engine state from the system clipboard, reloading the saved files in the process.
   * Requires a build with the clip module, logs an error otherwise.
   */
  void LoadStatefileFromClipboard();

  /**
   * Apply a parsed statefile (options, files, camera and window size) to the engine, replacing the
   * current state.
   * Used by LoadStatefile and LoadStatefileFromClipboard.
   */
  void ApplyStatefile(const std::map<std::string, std::string>& statefileOptions,
    const std::vector<std::string>& statefileFiles,
    const std::optional<StatefileFileGroups>& statefileFileGroups,
    const std::optional<std::pair<int, int>>& statefileWindowSize);

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
  void LoadFileGroupInternal(
    const std::vector<std::filesystem::path>& paths, bool clear, const std::string& groupIdx);

  /**
   * Internal event loop that is triggered repeatedly to handle specific events:
   * - Render
   * - ReloadFile
   */
  void EventLoop();

  /**
   * Resets the window name for f3d app
   */
  void ResetWindowName();

  /**
   * Add F3D specific commands
   */
  void AddCommands();
};

#endif

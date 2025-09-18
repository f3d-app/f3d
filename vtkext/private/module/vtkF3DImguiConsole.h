/**
 * @class   vtkF3DImguiConsole
 * @brief   An ImGui console window
 *
 * This class is used instead of vtkF3DConsoleOutputWindow if F3D_MODULE_UI is enabled
 * On top of the regular behavior of printing the log in the console, all the logs are also added
 * in an imgui window so the user can access it easily by calling ShowConsole()
 * It is also adding an input widget where commands registered in libf3d can be executed.
 * Finally, a small icon is displayed on the top right corner when the console is hidden but a new
 * warning or error is logged.
 */

#ifndef vtkF3DImguiConsole_h
#define vtkF3DImguiConsole_h

#include "vtkF3DConsoleOutputWindow.h"

#include <vtkCommand.h>

#include <functional>
#include <memory>

class vtkOpenGLRenderWindow;
class vtkWindow;
struct ImVec2;

class vtkF3DImguiConsole : public vtkF3DConsoleOutputWindow
{
public:
  static vtkF3DImguiConsole* New();
  vtkTypeMacro(vtkF3DImguiConsole, vtkF3DConsoleOutputWindow);

  /**
   * Add text to console
   */
  void DisplayText(const char*) override;

  /**
   * Show console window
   */
  void ShowConsole(bool);

  /**
   * Show console badge
   */
  void ShowBadge();

  /**
   * Clear console
   */
  void Clear();

  /**
   * Get Badge window size
   */
  ImVec2 GetBadgeSize();

  /**
   * Set the callback to get completion candidates
   */
  void SetCompletionCallback(
    std::function<std::vector<std::string>(const std::string& pattern)> callback);

protected:
  vtkF3DImguiConsole();
  ~vtkF3DImguiConsole() override;

private:
  struct Internals;
  std::unique_ptr<Internals> Pimpl;

private:
  vtkF3DImguiConsole(const vtkF3DImguiConsole&) = delete;
  void operator=(const vtkF3DImguiConsole&) = delete;
};

#endif

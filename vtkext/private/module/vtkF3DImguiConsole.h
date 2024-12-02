/**
 * @class   vtkF3DImguiConsole
 * @brief   An ImGui console window
 *
 * This class is used instead of vtkF3DConsoleOutputWindow if F3D_MODULE_UI is enabled
 */

#ifndef vtkF3DImguiConsole_h
#define vtkF3DImguiConsole_h

#include "vtkF3DConsoleOutputWindow.h"

#include <vtkCommand.h>

#include <memory>

class vtkOpenGLRenderWindow;
class vtkWindow;

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
  void ShowConsole();

  /**
   * Show console badge
   */
  void ShowBadge();

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

/**
 * @class   vtkF3DAndroidLogOutputWindow
 * @brief   Custom log output window for android
 *
 */
#ifndef vtkF3DAndroidLogOutputWindow_h
#define vtkF3DAndroidLogOutputWindow_h

#include "vtkOutputWindow.h"

class vtkF3DAndroidLogOutputWindow : public vtkOutputWindow
{
public:
  vtkTypeMacro(vtkF3DAndroidLogOutputWindow, vtkOutputWindow);
  static vtkF3DAndroidLogOutputWindow* New();

  /**
   * Send log messages to the Android internal logging system using __android_log_print
   * The log appears in the Logcat window when debugging the application using Android Studio
   */
  void DisplayText(const char*) override;

protected:
  vtkF3DAndroidLogOutputWindow();
  ~vtkF3DAndroidLogOutputWindow() override = default;

private:
  vtkF3DAndroidLogOutputWindow(const vtkF3DAndroidLogOutputWindow&) = delete;
  void operator=(const vtkF3DAndroidLogOutputWindow&) = delete;
};

#endif

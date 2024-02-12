#include "vtkF3DUIRenderWindow.h"

#include <vtkObjectFactory.h>

vtkStandardNewMacro(vtkF3DUIRenderWindow);

vtkF3DUIRenderWindow::vtkF3DUIRenderWindow()
: WindowId(nullptr)
{
  this->Initialize();
}

vtkF3DUIRenderWindow::~vtkF3DUIRenderWindow()
{
}

void vtkF3DUIRenderWindow::Initialize()
{
  if (!glfwInit()) {
    this->ReadyForRendering = false;
    vtkErrorMacro(<< "Error initializing GLFW3");
  }

  glfwWindowHint(GLFW_VERSION_MAJOR, 3);
  glfwWindowHint(GLFW_VERSION_MINOR, 2);

  this->ReadyForRendering = true;
}

void vtkF3DUIRenderWindow::CreateAWindow()
{
  this->WindowId = glfwCreateWindow(600, 600, "ImGUI prototype", nullptr, nullptr);

  if (!this->WindowId) {
    vtkErrorMacro(<< "Failed to create a glfw window");
  }

  glfwMakeContextCurrent(this->WindowId);
  glfwSwapInterval(1);

  GLFWmonitor* mon = glfwGetPrimaryMonitor();
  const GLFWvidmode* mode = glfwGetVideoMode(mon);
  int wmm(0), hmm(0);
  glfwGetMonitorPhysicalSize(mon, &wmm, &hmm);
  float dpi = (float)mode->width * (float)mode->height /
              ((float)wmm * (float)hmm * 0.0393701f * 0.0393701f);
  float xs(0), ys(0);
  glfwGetMonitorContentScale(mon, &xs, &ys);
  vtkDebugMacro(<< "Pixels " << mode->width << "x" << mode->height);
  vtkDebugMacro(<< "Screen " << wmm << "x" << hmm << "(mm)");
  vtkDebugMacro(<< "Red" << mode->redBits);
  vtkDebugMacro(<< "Blue" << mode->blueBits);
  vtkDebugMacro(<< "Green" << mode->greenBits);
  vtkDebugMacro(<< "Refresh-Rate" << mode->refreshRate);
  vtkDebugMacro(<< "Native" << dpi << "DPI");
  vtkDebugMacro(<< "Current" << xs * dpi << "DPI");
  vtkDebugMacro(<< "x-scale" << xs);
  vtkDebugMacro(<< "y-scale" << ys);
  this->SetDPI(xs * dpi);
}

void vtkF3DUIRenderWindow::DestroyWindow()
{
  if (this->WindowId) {
    glfwDestroyWindow(this->WindowId);
  }
  glfwTerminate();
}

void vtkF3DUIRenderWindow::Render()
{
  if (this->ReadyForRendering) {
    if (!this->WindowId)
    {
      this->CreateAWindow();
    }
  }
}


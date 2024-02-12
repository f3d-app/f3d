#include "vtkF3DUIWindowInteractor.h"

#include <GLFW/glfw3.h>
#include <vtkObjectFactory.h>
#include <vtkRenderWindow.h>

vtkStandardNewMacro(vtkF3DUIWindowInteractor);

vtkF3DUIWindowInteractor::vtkF3DUIWindowInteractor()
{
}

vtkF3DUIWindowInteractor::~vtkF3DUIWindowInteractor()
{
}

void vtkF3DUIWindowInteractor::Start()
{
  if (!this->RenderWindow) {
    vtkErrorMacro(<< "No renderer defined!");
    return;
  }

  vtkRenderWindow* ren = this->RenderWindow;
  GLFWwindow* window = static_cast<GLFWwindow *>(ren->GetGenericWindowId());

  while (!glfwWindowShouldClose(window)) {
    glfwPollEvents();
  }
}

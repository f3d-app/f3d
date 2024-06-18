#include <vtkNew.h>
#include <vtkRenderWindow.h>
#include <vtkRenderer.h>
#include <vtkTestUtilities.h>

#include "vtkF3DHexagonalBokehBlurPass.h"
#include "vtkF3DRenderPass.h"
#include "vtkF3DUserRenderPass.h"

#include <iostream>

int TestF3DRenderPass(int argc, char* argv[])
{
  vtkNew<vtkF3DRenderPass> pass;
  pass->Print(std::cout);

  vtkNew<vtkF3DHexagonalBokehBlurPass> bokeh;
  bokeh->SetDelegatePass(pass);
  bokeh->Print(std::cout);

  bokeh->SetCircleOfConfusionRadius(30.f);

  if (bokeh->GetCircleOfConfusionRadius() != 30.f)
  {
    return EXIT_FAILURE;
  }

  vtkNew<vtkF3DUserRenderPass> user;
  user->SetDelegatePass(bokeh);
  user->Print(std::cout);

  vtkNew<vtkRenderer> renderer;
  renderer->SetPass(user);

  vtkNew<vtkRenderWindow> renWin;
  renWin->AddRenderer(renderer);
  renWin->OffScreenRenderingOn();

  // set valid shader
  user->SetUserShader("vec4 pixel(vec2 uv) { return vec4(uv, 0.0, 1.0); }");
  renWin->Render();

  // change to another valid shader
  user->SetUserShader("vec4 pixel(vec2 uv) { return vec4(uv, 1.0, 1.0); }");
  renWin->Render();

  // render a second time to check if recompilation skipping is working
  renWin->Render();

  return EXIT_SUCCESS;
}

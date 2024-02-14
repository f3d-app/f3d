#include <vtkNew.h>
#include <vtkTestUtilities.h>

#include "vtkF3DHexagonalBokehBlurPass.h"
#include "vtkF3DRenderPass.h"

#include <iostream>

int TestF3DRenderPass(int argc, char* argv[])
{
  vtkNew<vtkF3DRenderPass> pass;
  pass->Print(std::cout);

  vtkNew<vtkF3DHexagonalBokehBlurPass> bokeh;
  bokeh->Print(std::cout);

  bokeh->SetCircleOfConfusionRadius(30.f);

  if (bokeh->GetCircleOfConfusionRadius() != 30.f)
  {
    return EXIT_FAILURE;
  }

  return EXIT_SUCCESS;
}

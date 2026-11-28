#include "vtkF3DMetaImporter.h"

#include <vtkDoubleArray.h>
#include <vtkNew.h>
#include <vtkOBJImporter.h>
#include <vtkRenderWindow.h>
#include <vtkRenderer.h>
#include <vtksys/SystemTools.hxx>

#include <iostream>

int TestF3DMetaImporterAnimation(int argc, char* argv[])
{
  vtkNew<vtkF3DMetaImporter> importer;

  // Check importer error code path
  if (!importer->GetAnimationName(0).empty())
  {
    std::cerr << "Unexpected animation name that should be empty\n";
    return EXIT_FAILURE;
  }
  if (importer->IsAnimationEnabled(0))
  {
    std::cerr << "Unexpected enabled animation that should not be\n";
    return EXIT_FAILURE;
  }

  int nbTimeSteps;
  double timeRange[2];
  if (importer->GetTemporalInformation(0, timeRange, nbTimeSteps, nullptr))
  {
    std::cerr << "Unexpected enabled animation that should not be\n";
    return EXIT_FAILURE;
  }

  // Read a OBJ to cover code paths for importers without animations support
  vtkNew<vtkOBJImporter> importerOBJ;
  const std::string filename = std::string(argv[1]) + "data/world.obj";
  importerOBJ->SetFileName(filename.c_str());
  const std::string path = vtksys::SystemTools::GetFilenamePath(filename);
  importerOBJ->SetTexturePath(path.c_str());
  importer->AddImporter({ "foo", importerOBJ });

  vtkNew<vtkRenderWindow> window;
  const vtkNew<vtkRenderer> renderer;
  window->AddRenderer(renderer);
  importer->SetRenderWindow(window);
  importer->Update();

  if (importer->GetNumberOfAnimations() != 0)
  {
    std::cerr << "Unexpected number of animations\n";
    return EXIT_FAILURE;
  }

  if (importer->GetAnimationName(0) != "")
  {
    std::cerr << "Unexpected non-empty animation name\n";
    return EXIT_FAILURE;
  }

  importer->EnableAnimation(0);
  if (importer->IsAnimationEnabled(0))
  {
    std::cerr << "Unexpected enable animation name after enabling without animation available\n";
    return EXIT_FAILURE;
  }

  importer->DisableAnimation(0);
  if (importer->IsAnimationEnabled(0))
  {
    std::cerr << "Unexpected enable animation name after disabling without animation available\n";
    return EXIT_FAILURE;
  }

  importer->SetVirtualCameraOrbitPeriod(-4.0);
  if (importer->GetNumberOfAnimations() != 1)
  {
    std::cerr << "Unexpected number of animations with a virtual camera\n";
    return EXIT_FAILURE;
  }

  if (importer->GetAnimationSupportLevel() != vtkImporter::AnimationSupportLevel::UNIQUE)
  {
    std::cerr << "Unexpected animation support level with a virtual camera\n";
    return EXIT_FAILURE;
  }

  if (importer->GetAnimationName(0) != "Camera Orbit" || !importer->GetAnimationName(1).empty())
  {
    std::cerr << "Unexpected animation names with a virtual camera\n";
    return EXIT_FAILURE;
  }

  vtkNew<vtkDoubleArray> timeSteps;
  if (!importer->GetTemporalInformation(0, timeRange, nbTimeSteps, timeSteps) ||
    timeRange[0] != 0.0 || timeRange[1] != 4.0 || nbTimeSteps != 5 ||
    timeSteps->GetValue(1) != 1.0 || timeSteps->GetValue(4) != 4.0)
  {
    std::cerr << "Unexpected temporal information of the virtual animation\n";
    return EXIT_FAILURE;
  }

  if (importer->GetTemporalInformation(1, timeRange, nbTimeSteps, timeSteps))
  {
    std::cerr << "Unexpected temporal information for an animation that does not exist\n";
    return EXIT_FAILURE;
  }

  return EXIT_SUCCESS;
}

#include "vtkF3DMetaImporter.h"

#include <vtkCubeSource.h>
#include <vtkMathUtilities.h>
#include <vtkNew.h>
#include <vtkPolyDataMapper.h>
#include <vtkRenderWindow.h>
#include <vtkRenderer.h>

#include <iostream>

namespace
{
// Testing importer which creates a single cube actor with a known, off-center position and size,
// so that the effect of coordinate normalization can be checked precisely.
class OffsetCubeImporter : public vtkImporter
{
public:
  static OffsetCubeImporter* New();
  vtkTypeMacro(OffsetCubeImporter, vtkImporter);

  void ImportActors(vtkRenderer* renderer) override
  {
    vtkNew<vtkCubeSource> cube;
    cube->SetCenter(100.0, 200.0, -300.0);
    cube->SetXLength(10.0);
    cube->SetYLength(10.0);
    cube->SetZLength(10.0);
    cube->Update();

    vtkNew<vtkPolyDataMapper> mapper;
    mapper->SetInputData(cube->GetOutput());

    vtkNew<vtkActor> actor;
    actor->SetMapper(mapper);
    renderer->AddActor(actor);

    this->ActorCollection->AddItem(actor);
  }
};
vtkStandardNewMacro(OffsetCubeImporter);
}

int TestF3DMetaImporterCoordinateNormalize(int argc, char* argv[])
{
  vtkNew<vtkF3DMetaImporter> importer;
  const vtkNew<OffsetCubeImporter> importerCube;
  importer->AddImporter({ "foo", importerCube });

  vtkNew<vtkRenderWindow> window;
  const vtkNew<vtkRenderer> renderer;
  window->AddRenderer(renderer);
  importer->SetRenderWindow(window);

  // Without normalization, the reported bounding box should reflect the raw, off-center geometry
  importer->Update();
  const vtkBoundingBox& rawBBox = importer->GetGeometryBoundingBox();
  double rawCenter[3];
  rawBBox.GetCenter(rawCenter);
  if (!vtkMathUtilities::FuzzyCompare(rawCenter[0], 100.0, 1e-6) ||
    !vtkMathUtilities::FuzzyCompare(rawCenter[1], 200.0, 1e-6) ||
    !vtkMathUtilities::FuzzyCompare(rawCenter[2], -300.0, 1e-6))
  {
    std::cerr << "Unexpected raw bounding box center before normalization\n";
    return EXIT_FAILURE;
  }
  if (!vtkMathUtilities::FuzzyCompare(rawBBox.GetMaxLength(), 10.0, 1e-6))
  {
    std::cerr << "Unexpected raw bounding box max length before normalization\n";
    return EXIT_FAILURE;
  }

  // Enabling normalization and updating again should shift the scene to the origin and scale it
  // to a unit bounding box
  importer->SetCoordinateNormalize(true);
  importer->Update();

  const vtkBoundingBox& normalizedBBox = importer->GetGeometryBoundingBox();
  double normalizedCenter[3];
  normalizedBBox.GetCenter(normalizedCenter);
  if (!vtkMathUtilities::FuzzyCompare(normalizedCenter[0], 0.0, 1e-6) ||
    !vtkMathUtilities::FuzzyCompare(normalizedCenter[1], 0.0, 1e-6) ||
    !vtkMathUtilities::FuzzyCompare(normalizedCenter[2], 0.0, 1e-6))
  {
    std::cerr << "Unexpected bounding box center after normalization\n";
    return EXIT_FAILURE;
  }
  if (!vtkMathUtilities::FuzzyCompare(normalizedBBox.GetMaxLength(), 1.0, 1e-6))
  {
    std::cerr << "Unexpected bounding box max length after normalization\n";
    return EXIT_FAILURE;
  }

  // The actual actor geometry, not just the reported metadata, should be transformed too
  vtkActorCollection* actors = importer->GetImportedActors();
  vtkCollectionSimpleIterator ait;
  actors->InitTraversal(ait);
  vtkActor* actor = actors->GetNextActor(ait);
  if (!actor)
  {
    std::cerr << "Expected an actor to be present\n";
    return EXIT_FAILURE;
  }
  double actorBounds[6];
  actor->GetBounds(actorBounds);
  const double actorCenter[3] = { (actorBounds[0] + actorBounds[1]) / 2.0,
    (actorBounds[2] + actorBounds[3]) / 2.0, (actorBounds[4] + actorBounds[5]) / 2.0 };
  if (!vtkMathUtilities::FuzzyCompare(actorCenter[0], 0.0, 1e-6) ||
    !vtkMathUtilities::FuzzyCompare(actorCenter[1], 0.0, 1e-6) ||
    !vtkMathUtilities::FuzzyCompare(actorCenter[2], 0.0, 1e-6))
  {
    std::cerr << "Unexpected actor bounds center after normalization\n";
    return EXIT_FAILURE;
  }

  // Disabling normalization and updating again should restore the original, raw bounds
  importer->SetCoordinateNormalize(false);
  importer->Update();

  const vtkBoundingBox& restoredBBox = importer->GetGeometryBoundingBox();
  double restoredCenter[3];
  restoredBBox.GetCenter(restoredCenter);
  if (!vtkMathUtilities::FuzzyCompare(restoredCenter[0], 100.0, 1e-6) ||
    !vtkMathUtilities::FuzzyCompare(restoredCenter[1], 200.0, 1e-6) ||
    !vtkMathUtilities::FuzzyCompare(restoredCenter[2], -300.0, 1e-6))
  {
    std::cerr << "Unexpected bounding box center after disabling normalization\n";
    return EXIT_FAILURE;
  }

  return EXIT_SUCCESS;
}

#include "vtkF3DMetaImporter.h"

#include <vtkCamera.h>
#include <vtkCubeSource.h>
#include <vtkLight.h>
#include <vtkLightCollection.h>
#include <vtkMathUtilities.h>
#include <vtkNew.h>
#include <vtkPolyDataMapper.h>
#include <vtkRenderWindow.h>
#include <vtkRenderer.h>

#include <iostream>

namespace
{
// Testing importer which creates a single cube actor with a known, off-center position and size,
// along with a camera and a light, so that the effect of coordinate normalization can be checked
// precisely on every kind of object it touches.
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

  void ImportCameras(vtkRenderer*) override
  {
    vtkNew<vtkCamera> camera;
    camera->SetPosition(100.0, 200.0, -280.0);
    camera->SetFocalPoint(100.0, 200.0, -300.0);
    this->CameraCollection->AddItem(camera);
  }

  void ImportLights(vtkRenderer*) override
  {
    vtkNew<vtkLight> light;
    light->SetPositional(true);
    light->SetPosition(120.0, 200.0, -300.0);
    light->SetFocalPoint(100.0, 200.0, -300.0);
    this->LightCollection->AddItem(light);
  }
};
vtkStandardNewMacro(OffsetCubeImporter);

// Testing importer which creates a single degenerate (zero-size) actor, to check that
// normalization is safely skipped rather than dividing by zero.
class DegenerateImporter : public vtkImporter
{
public:
  static DegenerateImporter* New();
  vtkTypeMacro(DegenerateImporter, vtkImporter);

  void ImportActors(vtkRenderer* renderer) override
  {
    vtkNew<vtkCubeSource> cube;
    cube->SetCenter(50.0, 50.0, 50.0);
    cube->SetXLength(0.0);
    cube->SetYLength(0.0);
    cube->SetZLength(0.0);
    cube->Update();

    vtkNew<vtkPolyDataMapper> mapper;
    mapper->SetInputData(cube->GetOutput());

    vtkNew<vtkActor> actor;
    actor->SetMapper(mapper);
    renderer->AddActor(actor);

    this->ActorCollection->AddItem(actor);
  }
};
vtkStandardNewMacro(DegenerateImporter);
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

  // The imported camera should be shifted and scaled by the same transform: it was 20 units
  // away from the cube center along Z, so after scaling by 1/10 it should be 2 units away from
  // the origin along Z
  vtkCollection* cameras = importerCube->GetImportedCameras();
  vtkCollectionSimpleIterator cit;
  cameras->InitTraversal(cit);
  vtkCamera* camera = vtkCamera::SafeDownCast(cameras->GetNextItemAsObject(cit));
  if (!camera)
  {
    std::cerr << "Expected a camera to be present\n";
    return EXIT_FAILURE;
  }
  double cameraPosition[3];
  camera->GetPosition(cameraPosition);
  if (!vtkMathUtilities::FuzzyCompare(cameraPosition[0], 0.0, 1e-6) ||
    !vtkMathUtilities::FuzzyCompare(cameraPosition[1], 0.0, 1e-6) ||
    !vtkMathUtilities::FuzzyCompare(cameraPosition[2], 2.0, 1e-6))
  {
    std::cerr << "Unexpected camera position after normalization\n";
    return EXIT_FAILURE;
  }
  double cameraFocalPoint[3];
  camera->GetFocalPoint(cameraFocalPoint);
  if (!vtkMathUtilities::FuzzyCompare(cameraFocalPoint[0], 0.0, 1e-6) ||
    !vtkMathUtilities::FuzzyCompare(cameraFocalPoint[1], 0.0, 1e-6) ||
    !vtkMathUtilities::FuzzyCompare(cameraFocalPoint[2], 0.0, 1e-6))
  {
    std::cerr << "Unexpected camera focal point after normalization\n";
    return EXIT_FAILURE;
  }

  // The imported light should be transformed the same way: it was 20 units away from the cube
  // center along X, so after scaling by 1/10 it should be 2 units away from the origin along X
  vtkLightCollection* lights = importerCube->GetImportedLights();
  vtkCollectionSimpleIterator lit;
  lights->InitTraversal(lit);
  vtkLight* light = lights->GetNextLight(lit);
  if (!light)
  {
    std::cerr << "Expected a light to be present\n";
    return EXIT_FAILURE;
  }
  double lightPosition[3];
  light->GetPosition(lightPosition);
  if (!vtkMathUtilities::FuzzyCompare(lightPosition[0], 2.0, 1e-6) ||
    !vtkMathUtilities::FuzzyCompare(lightPosition[1], 0.0, 1e-6) ||
    !vtkMathUtilities::FuzzyCompare(lightPosition[2], 0.0, 1e-6))
  {
    std::cerr << "Unexpected light position after normalization\n";
    return EXIT_FAILURE;
  }

  // Disabling normalization and updating again should restore the original, raw bounds and
  // the original camera and light positions
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
  camera->GetPosition(cameraPosition);
  if (!vtkMathUtilities::FuzzyCompare(cameraPosition[2], -280.0, 1e-6))
  {
    std::cerr << "Unexpected camera position after disabling normalization\n";
    return EXIT_FAILURE;
  }

  // A scene with a zero-size bounding box should not crash and should be left untransformed
  vtkNew<vtkF3DMetaImporter> degenerateImporter;
  const vtkNew<DegenerateImporter> importerDegenerate;
  degenerateImporter->AddImporter({ "foo", importerDegenerate });

  vtkNew<vtkRenderWindow> degenerateWindow;
  const vtkNew<vtkRenderer> degenerateRenderer;
  degenerateWindow->AddRenderer(degenerateRenderer);
  degenerateImporter->SetRenderWindow(degenerateWindow);
  degenerateImporter->SetCoordinateNormalize(true);
  // Will crash or throw here if the degenerate bounding box is not handled safely.
  degenerateImporter->Update();

  const vtkBoundingBox& degenerateBBox = degenerateImporter->GetGeometryBoundingBox();
  double degenerateCenter[3];
  degenerateBBox.GetCenter(degenerateCenter);
  if (!vtkMathUtilities::FuzzyCompare(degenerateCenter[0], 50.0, 1e-6) ||
    !vtkMathUtilities::FuzzyCompare(degenerateCenter[1], 50.0, 1e-6) ||
    !vtkMathUtilities::FuzzyCompare(degenerateCenter[2], 50.0, 1e-6))
  {
    std::cerr << "Unexpected bounding box center for a degenerate scene\n";
    return EXIT_FAILURE;
  }

  return EXIT_SUCCESS;
}

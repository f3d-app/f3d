#include "vtkF3DMetaImporter.h"

#include <vtkCamera.h>
#include <vtkCubeSource.h>
#include <vtkLight.h>
#include <vtkLightCollection.h>
#include <vtkMathUtilities.h>
#include <vtkMatrix4x4.h>
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
    // Local origin. vtkLight's constructed position is (0, 0, 1); the world placement is the
    // matrix below, 20 units from the cube center along X.
    light->SetPosition(0.0, 0.0, 0.0);

    vtkNew<vtkMatrix4x4> lightMatrix;
    lightMatrix->SetElement(0, 3, 120.0);
    lightMatrix->SetElement(1, 3, 200.0);
    lightMatrix->SetElement(2, 3, -300.0);
    light->SetTransformMatrix(lightMatrix);
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

// Two cubes of length 10. The second is placed entirely by an actor matrix, translating it by
// 100 along X, so normalization has to include that matrix in the scene bounds and concatenate
// with it instead of replacing it.
class PlacedCubeImporter : public vtkImporter
{
public:
  static PlacedCubeImporter* New();
  vtkTypeMacro(PlacedCubeImporter, vtkImporter);

  void ImportActors(vtkRenderer* renderer) override
  {
    auto addCube = [&](vtkMatrix4x4* matrix)
    {
      vtkNew<vtkCubeSource> cube;
      cube->SetXLength(10.0);
      cube->SetYLength(10.0);
      cube->SetZLength(10.0);
      cube->Update();

      vtkNew<vtkPolyDataMapper> mapper;
      mapper->SetInputData(cube->GetOutput());

      vtkNew<vtkActor> actor;
      actor->SetMapper(mapper);
      if (matrix)
      {
        actor->SetUserMatrix(matrix);
      }
      renderer->AddActor(actor);
      this->ActorCollection->AddItem(actor);
    };

    addCube(nullptr);

    vtkNew<vtkMatrix4x4> translated;
    translated->SetElement(0, 3, 100.0);
    addCube(translated);
  }
};
vtkStandardNewMacro(PlacedCubeImporter);

// Cube matching OffsetCubeImporter, plus one camera whose source position is rewritten on each
// animation sample. z = -280 - 20*t, so t=0 is 20 units in front of the cube and t=1 sits on
// the cube center.
class AnimatedCameraImporter : public vtkImporter
{
public:
  static AnimatedCameraImporter* New();
  vtkTypeMacro(AnimatedCameraImporter, vtkImporter);

  vtkCamera* Camera = nullptr;

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
    this->Camera = camera;
    this->CameraCollection->AddItem(camera);
    this->UpdateAtTimeValue(0.0);
  }

  bool UpdateAtTimeValue(double timeValue) override
  {
    if (this->Camera)
    {
      this->Camera->SetPosition(100.0, 200.0, -280.0 - timeValue * 20.0);
      this->Camera->SetFocalPoint(100.0, 200.0, -300.0);
    }
    return true;
  }
};
vtkStandardNewMacro(AnimatedCameraImporter);
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

  // The light's world position is its transform matrix applied to the local origin: the matrix
  // translation (120, 200, -300), 20 units from the cube center along X.
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
  light->GetTransformedPosition(lightPosition);
  if (!vtkMathUtilities::FuzzyCompare(lightPosition[0], 120.0, 1e-6) ||
    !vtkMathUtilities::FuzzyCompare(lightPosition[1], 200.0, 1e-6) ||
    !vtkMathUtilities::FuzzyCompare(lightPosition[2], -300.0, 1e-6))
  {
    std::cerr << "Unexpected light position before normalization\n";
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

  // Same light, now through the normalization transform: (120, 200, -300) is 20 units from the
  // cube center along X, and the scene scale is 1/10, so the world position is (2, 0, 0).
  light->GetTransformedPosition(lightPosition);
  if (!vtkMathUtilities::FuzzyCompare(lightPosition[0], 2.0, 1e-6) ||
    !vtkMathUtilities::FuzzyCompare(lightPosition[1], 0.0, 1e-6) ||
    !vtkMathUtilities::FuzzyCompare(lightPosition[2], 0.0, 1e-6))
  {
    std::cerr << "Unexpected light position after normalization\n";
    return EXIT_FAILURE;
  }

  // A camera imported after normalization is already on is still in source coordinates. It must
  // receive the full matrix. The scene bounds do not change, so the delta from the previous
  // matrix is identity and would leave this camera untransformed.
  const vtkNew<OffsetCubeImporter> importerCubeAgain;
  importer->AddImporter({ "bar", importerCubeAgain });
  importer->Update();

  vtkCollection* camerasAgain = importerCubeAgain->GetImportedCameras();
  vtkCollectionSimpleIterator citAgain;
  camerasAgain->InitTraversal(citAgain);
  vtkCamera* cameraAgain = vtkCamera::SafeDownCast(camerasAgain->GetNextItemAsObject(citAgain));
  if (!cameraAgain)
  {
    std::cerr << "Expected a camera on the second importer\n";
    return EXIT_FAILURE;
  }
  double cameraAgainPosition[3];
  cameraAgain->GetPosition(cameraAgainPosition);
  if (!vtkMathUtilities::FuzzyCompare(cameraAgainPosition[0], 0.0, 1e-6) ||
    !vtkMathUtilities::FuzzyCompare(cameraAgainPosition[1], 0.0, 1e-6) ||
    !vtkMathUtilities::FuzzyCompare(cameraAgainPosition[2], 2.0, 1e-6))
  {
    std::cerr << "Unexpected camera position for a camera imported after normalization\n";
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
  light->GetTransformedPosition(lightPosition);
  if (!vtkMathUtilities::FuzzyCompare(lightPosition[0], 120.0, 1e-6) ||
    !vtkMathUtilities::FuzzyCompare(lightPosition[1], 200.0, 1e-6) ||
    !vtkMathUtilities::FuzzyCompare(lightPosition[2], -300.0, 1e-6))
  {
    std::cerr << "Unexpected light position after disabling normalization\n";
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

  // A cube translated only by its actor matrix must move with the rest of the scene. The raw
  // bounds run from x=-5 to x=105, so the max length is 110 and the center is (50, 0, 0).
  vtkNew<vtkF3DMetaImporter> placedImporter;
  const vtkNew<PlacedCubeImporter> importerPlaced;
  placedImporter->AddImporter({ "placed", importerPlaced });

  vtkNew<vtkRenderWindow> placedWindow;
  const vtkNew<vtkRenderer> placedRenderer;
  placedWindow->AddRenderer(placedRenderer);
  placedImporter->SetRenderWindow(placedWindow);
  placedImporter->Update();

  const vtkBoundingBox& placedRawBBox = placedImporter->GetGeometryBoundingBox();
  if (!vtkMathUtilities::FuzzyCompare(placedRawBBox.GetMaxLength(), 110.0, 1e-6))
  {
    std::cerr << "Unexpected raw max length for a scene placed by an actor matrix\n";
    return EXIT_FAILURE;
  }

  placedImporter->SetCoordinateNormalize(true);
  placedImporter->Update();

  vtkActorCollection* placedActors = placedImporter->GetImportedActors();
  vtkCollectionSimpleIterator placedIt;
  placedActors->InitTraversal(placedIt);
  vtkActor* stationary = placedActors->GetNextActor(placedIt);
  vtkActor* translated = placedActors->GetNextActor(placedIt);
  if (!stationary || !translated)
  {
    std::cerr << "Expected two actors in the placed scene\n";
    return EXIT_FAILURE;
  }

  auto centerOf = [](vtkActor* actor, double center[3])
  {
    double bounds[6];
    actor->GetBounds(bounds);
    center[0] = (bounds[0] + bounds[1]) / 2.0;
    center[1] = (bounds[2] + bounds[3]) / 2.0;
    center[2] = (bounds[4] + bounds[5]) / 2.0;
  };

  const double fifth = 5.0 / 11.0;
  double stationaryCenter[3];
  double translatedCenter[3];
  centerOf(stationary, stationaryCenter);
  centerOf(translated, translatedCenter);
  if (!vtkMathUtilities::FuzzyCompare(stationaryCenter[0], -fifth, 1e-6) ||
    !vtkMathUtilities::FuzzyCompare(stationaryCenter[1], 0.0, 1e-6) ||
    !vtkMathUtilities::FuzzyCompare(stationaryCenter[2], 0.0, 1e-6) ||
    !vtkMathUtilities::FuzzyCompare(translatedCenter[0], fifth, 1e-6) ||
    !vtkMathUtilities::FuzzyCompare(translatedCenter[1], 0.0, 1e-6) ||
    !vtkMathUtilities::FuzzyCompare(translatedCenter[2], 0.0, 1e-6))
  {
    std::cerr << "Unexpected actor centers after normalizing a scene placed by an actor matrix\n";
    return EXIT_FAILURE;
  }

  const vtkBoundingBox& placedNormalizedBBox = placedImporter->GetGeometryBoundingBox();
  if (!vtkMathUtilities::FuzzyCompare(placedNormalizedBBox.GetMaxLength(), 1.0, 1e-6))
  {
    std::cerr << "Unexpected normalized max length for a scene placed by an actor matrix\n";
    return EXIT_FAILURE;
  }

  placedImporter->SetCoordinateNormalize(false);
  placedImporter->Update();
  centerOf(stationary, stationaryCenter);
  centerOf(translated, translatedCenter);
  if (!vtkMathUtilities::FuzzyCompare(stationaryCenter[0], 0.0, 1e-6) ||
    !vtkMathUtilities::FuzzyCompare(translatedCenter[0], 100.0, 1e-6))
  {
    std::cerr << "Unexpected actor centers after disabling normalization of a placed scene\n";
    return EXIT_FAILURE;
  }

  // Animation rewrites the camera in source coordinates on every sample. Normalization has to be
  // applied again afterwards. The cube is centered at (100, 200, -300) with max length 10, so a
  // source position (100, 200, z) becomes (0, 0, (z + 300) / 10).
  vtkNew<vtkF3DMetaImporter> animatedImporter;
  const vtkNew<AnimatedCameraImporter> importerAnimated;
  animatedImporter->AddImporter({ "animated", importerAnimated });

  vtkNew<vtkRenderWindow> animatedWindow;
  const vtkNew<vtkRenderer> animatedRenderer;
  animatedWindow->AddRenderer(animatedRenderer);
  animatedImporter->SetRenderWindow(animatedWindow);
  animatedImporter->SetCoordinateNormalize(true);
  animatedImporter->Update();

  animatedImporter->UpdateAtTimeValue(0.0);
  if (!importerAnimated->Camera)
  {
    std::cerr << "Expected an animated camera to be present\n";
    return EXIT_FAILURE;
  }
  double animatedPosition[3];
  importerAnimated->Camera->GetPosition(animatedPosition);
  if (!vtkMathUtilities::FuzzyCompare(animatedPosition[0], 0.0, 1e-6) ||
    !vtkMathUtilities::FuzzyCompare(animatedPosition[1], 0.0, 1e-6) ||
    !vtkMathUtilities::FuzzyCompare(animatedPosition[2], 2.0, 1e-6))
  {
    std::cerr << "Unexpected camera position after normalizing animation frame 0\n";
    return EXIT_FAILURE;
  }

  animatedImporter->UpdateAtTimeValue(1.0);
  importerAnimated->Camera->GetPosition(animatedPosition);
  if (!vtkMathUtilities::FuzzyCompare(animatedPosition[0], 0.0, 1e-6) ||
    !vtkMathUtilities::FuzzyCompare(animatedPosition[1], 0.0, 1e-6) ||
    !vtkMathUtilities::FuzzyCompare(animatedPosition[2], 0.0, 1e-6))
  {
    std::cerr << "Unexpected camera position after normalizing animation frame 1\n";
    return EXIT_FAILURE;
  }

  return EXIT_SUCCESS;
}

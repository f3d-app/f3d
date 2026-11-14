#include "F3DSparseMorphingTestUtilities.h"

#include "F3DLog.h"
#include "vtkF3DImporter.h"
#include "vtkF3DMetaImporter.h"
#include "vtkF3DPolyDataMapper.h"
#include "vtkF3DRenderer.h"

#include <vtkActorCollection.h>
#include <vtkBoundingBox.h>
#include <vtkFieldData.h>
#include <vtkFloatArray.h>
#include <vtkIntArray.h>
#include <vtkMatrix4x4.h>
#include <vtkPointData.h>
#include <vtkPoints.h>
#include <vtkPolyData.h>
#include <vtkRenderWindow.h>
#include <vtkShader.h>

#include <cmath>
#include <iostream>

namespace
{
class MorphingImporter : public vtkF3DImporter
{
public:
  static MorphingImporter* New()
  {
    return new MorphingImporter;
  }
  bool UpdateAtTimeValue(double time) override
  {
    this->LastGPU = this->GetUseGPUMorphing();
    this->LastTime = time;
    return this->Succeed;
  }
  bool Succeed = true;
  bool LastGPU = false;
  double LastTime = -1.;
  vtkSmartPointer<vtkActor> Actor;

protected:
  int ImportBegin() override
  {
    return 1;
  }
  void ImportActors(vtkRenderer* renderer) override
  {
    this->ActorCollection->AddItem(this->Actor);
    renderer->AddActor(this->Actor);
  }
};
}

int TestF3DMetaImporterMorphing(int, char*[])
{
  vtkNew<vtkRenderWindow> window;
  window->OffScreenRenderingOn();
  vtkNew<vtkF3DRenderer> renderer;
  window->AddRenderer(renderer);
  window->Start();
  if (!F3DTest::SupportsSparseMorphing(window))
  {
    std::cout << "Sparse morphing is unavailable; skipping GPU mode test\n";
    return EXIT_SUCCESS;
  }
  window->Render();
  vtkNew<vtkPolyData> polydata;
  vtkNew<vtkPoints> points;
  points->InsertNextPoint(0., 0., 0.);
  polydata->SetPoints(points);
  vtkNew<vtkFloatArray> bind;
  bind->DeepCopy(points->GetData());
  bind->SetName("blendShapeBindPositions");
  polydata->GetPointData()->AddArray(bind);
  vtkNew<vtkIntArray> ranges;
  ranges->SetName("blendShapeRanges");
  ranges->SetNumberOfComponents(2);
  ranges->InsertNextTuple2(0, 1);
  polydata->GetPointData()->AddArray(ranges);
  vtkNew<vtkFloatArray> offsets;
  offsets->SetName("blendShapeOffsets");
  offsets->SetNumberOfComponents(4);
  offsets->InsertNextTuple4(0., 1., 0., 0.);
  polydata->GetFieldData()->AddArray(offsets);
  vtkNew<vtkFloatArray> weights;
  weights->SetName("blendShapeWeights");
  weights->InsertNextValue(0.5f);
  polydata->GetFieldData()->AddArray(weights);
  vtkNew<vtkF3DPolyDataMapper> mapper;
  mapper->SetInputData(polydata);
  vtkNew<vtkActor> actor;
  actor->SetMapper(mapper);
  vtkNew<MorphingImporter> source;
  source->Actor = actor;
  vtkNew<vtkF3DMetaImporter> importer;
  importer->AddImporter({ "morphing", source });
  importer->SetRenderWindow(window);
  if (!importer->Update() || !importer->UpdateAtTimeValue(0.75) || !source->LastGPU)
  {
    return EXIT_FAILURE;
  }
  // Arbitrary up directions must include the GPU deformation in oriented scene bounds.
  vtkNew<vtkMatrix4x4> basis;
  const double diagonal = std::sqrt(0.5);
  basis->SetElement(0, 0, diagonal);
  basis->SetElement(0, 1, -diagonal);
  basis->SetElement(1, 0, diagonal);
  basis->SetElement(1, 1, diagonal);
  double bounds[6];
  renderer->ComputeVisiblePropOrientedBounds(basis).GetBounds(bounds);
  if (std::abs(bounds[0] + 0.5 * diagonal) > 1e-6 || std::abs(bounds[2] - 0.5 * diagonal) > 1e-6)
  {
    return EXIT_FAILURE;
  }
  renderer->SetUsePointSprites(true);
  if (!importer->RefreshMorphingMode() || source->LastGPU || source->LastTime != 0.75)
  {
    return EXIT_FAILURE;
  }
  renderer->SetUsePointSprites(false);
  if (!importer->RefreshMorphingMode() || !source->LastGPU)
  {
    return EXIT_FAILURE;
  }
  renderer->SetRenderingMode(vtkF3DRenderer::RenderingMode::OSPRAY);
  if (!importer->RefreshMorphingMode() || source->LastGPU)
  {
    return EXIT_FAILURE;
  }
  // A failed importer update during a mode transition must be reported, then recover.
  renderer->SetImporter(importer);
  renderer->SetRenderingMode(vtkF3DRenderer::RenderingMode::OPENGL);
  source->Succeed = false;
  bool warningSeen = false;
  const auto previousForwarder = F3DLog::Forwarder;
  F3DLog::Forward(
    [&](F3DLog::Severity severity, const std::string& message)
    {
      warningSeen = warningSeen ||
        (severity == F3DLog::Severity::Warning && message == "Failed to update morphing mode");
    });
  window->Render();
  F3DLog::Forward(previousForwarder);
  source->Succeed = true;
  const bool recovered = importer->UpdateAtTimeValue(0.75);
  renderer->SetImporter(nullptr);
  if (!warningSeen || !recovered || !source->LastGPU || source->LastTime != 0.75)
  {
    std::cerr << "Failed morphing transition was not reported or could not recover\n";
    return EXIT_FAILURE;
  }
  mapper->ReleaseGraphicsResources(window);
  return EXIT_SUCCESS;
}

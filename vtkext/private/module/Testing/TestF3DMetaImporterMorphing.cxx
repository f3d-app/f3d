#include "vtkF3DImporter.h"
#include "vtkF3DMetaImporter.h"
#include "vtkF3DPolyDataMapper.h"
#include "vtkF3DRenderer.h"

#include <vtkActorCollection.h>
#include <vtkFieldData.h>
#include <vtkFloatArray.h>
#include <vtkIntArray.h>
#include <vtkPointData.h>
#include <vtkPoints.h>
#include <vtkPolyData.h>
#include <vtkRenderWindow.h>
#include <vtkShader.h>

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
    return true;
  }
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
  window->Render();
  if (!vtkShader::IsComputeShaderSupported())
  {
    return EXIT_SUCCESS;
  }
  vtkNew<vtkF3DRenderer> renderer;
  window->AddRenderer(renderer);
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
  mapper->ReleaseGraphicsResources(window);
  return EXIT_SUCCESS;
}

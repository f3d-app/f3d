#include "vtkF3DUSDImporter.h"

#include <vtkActor.h>
#include <vtkActorCollection.h>
#include <vtkCallbackCommand.h>
#include <vtkCommand.h>
#include <vtkFieldData.h>
#include <vtkFloatArray.h>
#include <vtkIdTypeArray.h>
#include <vtkIntArray.h>
#include <vtkNew.h>
#include <vtkOpenGLRenderWindow.h>
#include <vtkPointData.h>
#include <vtkPoints.h>
#include <vtkPolyData.h>
#include <vtkPolyDataMapper.h>
#include <vtkRenderWindow.h>
#include <vtkRenderer.h>
#include <vtk_glad.h>

#include <cmath>
#include <iostream>
#include <string>

namespace
{
// A non-OpenGL window exercises the importer's capability-based CPU fallback.
class CPUWindow : public vtkRenderWindow
{
public:
  static CPUWindow* New()
  {
    return new CPUWindow;
  }
  vtkTypeMacro(CPUWindow, vtkRenderWindow);
};

vtkSmartPointer<vtkF3DUSDImporter> CreateImporter(vtkRenderWindow* window)
{
  auto importer = vtkSmartPointer<vtkF3DUSDImporter>::New();
  if (CPUWindow::SafeDownCast(window))
  {
    const vtkNew<CPUWindow> cpuWindow;
    importer->SetRenderWindow(cpuWindow);
  }
  else
  {
    importer->SetRenderWindow(window);
  }
  return importer;
}

bool CheckPackedDeformation(vtkPolyData* polydata)
{
  auto* offsets =
    vtkFloatArray::SafeDownCast(polydata->GetFieldData()->GetArray("blendShapeOffsets"));
  auto* weights =
    vtkFloatArray::SafeDownCast(polydata->GetFieldData()->GetArray("blendShapeWeights"));
  auto* ranges = vtkIntArray::SafeDownCast(polydata->GetPointData()->GetArray("blendShapeRanges"));
  auto* sourceIds = vtkIdTypeArray::SafeDownCast(polydata->GetPointData()->GetArray("SourceIds"));
  auto* bindPositions =
    vtkFloatArray::SafeDownCast(polydata->GetPointData()->GetArray("blendShapeBindPositions"));
  if (!offsets || !weights || !ranges || !bindPositions ||
    bindPositions->GetNumberOfComponents() != 3 ||
    bindPositions->GetNumberOfTuples() != polydata->GetNumberOfPoints())
  {
    return false;
  }

  // Undeformed vertices shared by the three box fixtures.
  const double positions[8][3] = { { -0.5, -0.5, 0.5 }, { 0.5, -0.5, 0.5 }, { 0.5, 0.5, 0.5 },
    { -0.5, 0.5, 0.5 }, { -0.5, -0.5, -0.5 }, { 0.5, -0.5, -0.5 }, { 0.5, 0.5, -0.5 },
    { -0.5, 0.5, -0.5 } };
  for (vtkIdType i = 0; i < polydata->GetNumberOfPoints(); i++)
  {
    const vtkIdType source = sourceIds ? sourceIds->GetValue(i) : i;
    if (source < 0 || source >= 8)
    {
      return false;
    }
    int range[2] = {};
    ranges->GetTypedTuple(i, range);
    if (range[0] < 0 || range[1] < range[0] || range[1] > offsets->GetNumberOfTuples())
    {
      return false;
    }
    double position[3];
    bindPositions->GetTuple(i, position);
    for (int component = 0; component < 3; component++)
    {
      if (position[component] != positions[source][component])
      {
        std::cerr << "Bind position changed during animation at vertex " << i << '\n';
        return false;
      }
    }
    for (int j = range[0]; j < range[1]; j++)
    {
      float offset[4] = {};
      offsets->GetTypedTuple(j, offset);
      const vtkIdType subshape = static_cast<vtkIdType>(offset[3]);
      if (subshape < 0 || subshape >= weights->GetNumberOfValues())
      {
        return false;
      }
      for (int component = 0; component < 3; component++)
      {
        position[component] += offset[component] * weights->GetValue(subshape);
      }
    }
    double actual[3];
    polydata->GetPoint(i, actual);
    for (int component = 0; component < 3; component++)
    {
      if (std::abs(position[component] - actual[component]) > 1e-6)
      {
        std::cerr << "Packed deformation differs from CPU at vertex " << i << '\n';
        return false;
      }
    }
  }
  return true;
}
}

int TestF3DUSDImporterBlendShapes(int vtkNotUsed(argc), char* argv[])
{
  const vtkNew<CPUWindow> cpuWindow;
  vtkNew<vtkRenderWindow> gpuWindow;
  gpuWindow->OffScreenRenderingOn();
  gpuWindow->Start();
  bool gpuSupported = false;
#ifndef GL_ES_VERSION_3_0
  if (auto* context = vtkOpenGLRenderWindow::SafeDownCast(gpuWindow))
  {
    int major = 0, minor = 0;
    context->GetOpenGLVersion(major, minor);
    if (major > 4 || (major == 4 && minor >= 3))
    {
      int blocks = 0;
      glGetIntegerv(GL_MAX_VERTEX_SHADER_STORAGE_BLOCKS, &blocks);
      gpuSupported = blocks >= 3;
    }
  }
#endif
  gpuWindow->End();
  for (const bool faceVarying : { false, true })
  {
    const std::string filename = std::string(argv[1]) + "data/" +
      (faceVarying ? "SimpleBlendShapesFaceVarying.usda" : "SimpleBlendShapes.usda");
    auto importer = ::CreateImporter(cpuWindow);
    importer->SetFileName(filename.c_str());
    importer->Update();

    vtkActor* actor = importer->GetRenderer()->GetActors()->GetLastActor();
    auto* mapper = actor ? vtkPolyDataMapper::SafeDownCast(actor->GetMapper()) : nullptr;
    vtkPolyData* polydata = mapper ? mapper->GetInput() : nullptr;
    if (!polydata || polydata->GetNumberOfPoints() != (faceVarying ? 24 : 8))
    {
      std::cerr << "Unexpected blend shape geometry: " << filename << '\n';
      return EXIT_FAILURE;
    }

    auto* offsets =
      vtkFloatArray::SafeDownCast(polydata->GetFieldData()->GetArray("blendShapeOffsets"));
    auto* ranges =
      vtkIntArray::SafeDownCast(polydata->GetPointData()->GetArray("blendShapeRanges"));
    auto* sourceIds = vtkIdTypeArray::SafeDownCast(polydata->GetPointData()->GetArray("SourceIds"));
    if (!offsets || offsets->GetNumberOfComponents() != 4 || offsets->GetNumberOfTuples() != 8 ||
      !ranges || ranges->GetNumberOfComponents() != 2 ||
      ranges->GetNumberOfTuples() != polydata->GetNumberOfPoints() || (faceVarying && !sourceIds))
    {
      std::cerr << "Missing packed blend shape data: " << filename << '\n';
      return EXIT_FAILURE;
    }

    for (vtkIdType i = 0; i < polydata->GetNumberOfPoints(); i++)
    {
      const vtkIdType source = sourceIds ? sourceIds->GetValue(i) : i;
      int range[2] = {};
      ranges->GetTypedTuple(i, range);
      if (range[0] != source || range[1] != source + 1)
      {
        std::cerr << "Blend shape range does not follow the source vertex\n";
        return EXIT_FAILURE;
      }
      float offset[4] = {};
      offsets->GetTypedTuple(range[0], offset);
      if (offset[0] != 0.f || std::abs(offset[1] - 0.2f) > 1e-6f || offset[2] != 0.f ||
        offset[3] != (source < 4 ? 1.f : 3.f))
      {
        std::cerr << "Unexpected packed blend shape offset\n";
        return EXIT_FAILURE;
      }
    }

    // Animation order is reversed relative to the mesh's blend shape order.
    for (const double frame : { 1., 12.5, 24., 36., 48., 1. })
    {
      if (!importer->UpdateAtTimeValue(frame / 24.))
      {
        return EXIT_FAILURE;
      }
      auto* weights =
        vtkFloatArray::SafeDownCast(polydata->GetFieldData()->GetArray("blendShapeWeights"));
      const double firstWeight = frame <= 24. ? (frame - 1.) / 23. : (48. - frame) / 24.;
      if (!weights || weights->GetNumberOfComponents() != 1 || weights->GetNumberOfValues() != 4 ||
        weights->GetValue(0) != 0.f || weights->GetValue(2) != 0.f ||
        std::abs(weights->GetValue(1) - firstWeight) > 1e-6 ||
        std::abs(weights->GetValue(3) - (1. - firstWeight)) > 1e-6)
      {
        std::cerr << "Unexpected packed blend shape weights at frame " << frame << '\n';
        return EXIT_FAILURE;
      }
      if (!::CheckPackedDeformation(polydata))
      {
        return EXIT_FAILURE;
      }
      const vtkMTimeType pointsTime = polydata->GetPoints()->GetMTime();
      const vtkMTimeType weightsTime = weights->GetMTime();
      if (!importer->UpdateAtTimeValue(frame / 24.) ||
        pointsTime != polydata->GetPoints()->GetMTime() || weightsTime != weights->GetMTime() ||
        !::CheckPackedDeformation(polydata))
      {
        std::cerr << "Unchanged weights recomputed the output points\n";
        return EXIT_FAILURE;
      }
      // External edits must invalidate the cached deformation even at the same time.
      polydata->GetPoints()->SetPoint(0, 100., 100., 100.);
      polydata->GetPoints()->Modified();
      if (!importer->UpdateAtTimeValue(frame / 24.) || !::CheckPackedDeformation(polydata))
      {
        return EXIT_FAILURE;
      }
    }
    if (sourceIds)
    {
      const vtkIdType originalSource = sourceIds->GetValue(0);
      sourceIds->SetValue(0, -1);
      sourceIds->Modified();
      polydata->GetPoints()->SetPoint(0, 100., 100., 100.);
      polydata->GetPoints()->Modified();
      if (!importer->UpdateAtTimeValue(12.5 / 24.) || polydata->GetPoint(0)[0] != 100.)
      {
        std::cerr << "Invalid source ID accessed blend shape positions\n";
        return EXIT_FAILURE;
      }
      sourceIds->SetValue(0, originalSource);
      sourceIds->Modified();
      if (!importer->UpdateAtTimeValue(12.5 / 24.) || !::CheckPackedDeformation(polydata))
      {
        return EXIT_FAILURE;
      }
    }
  }

  auto importer = ::CreateImporter(cpuWindow);
  const std::string filename = std::string(argv[1]) + "data/SimpleBlendShapesInbetween.usda";
  importer->SetFileName(filename.c_str());
  importer->Update();
  for (const double frame : { 1., 12.5, 24., 36., 48., 1. })
  {
    if (!importer->UpdateAtTimeValue(frame / 24.))
    {
      return EXIT_FAILURE;
    }
    vtkActor* actor = importer->GetRenderer()->GetActors()->GetLastActor();
    auto* mapper = actor ? vtkPolyDataMapper::SafeDownCast(actor->GetMapper()) : nullptr;
    auto* weights = mapper ? vtkFloatArray::SafeDownCast(
                               mapper->GetInput()->GetFieldData()->GetArray("blendShapeWeights"))
                           : nullptr;
    const double firstWeight = frame <= 24. ? (frame - 1.) / 23. : (48. - frame) / 24.;
    const double halfWeight = firstWeight <= 0.5 ? 2. * firstWeight : 2. * (1. - firstWeight);
    const double primaryWeight = firstWeight <= 0.5 ? 0. : 2. * firstWeight - 1.;
    if (!weights || weights->GetNumberOfValues() != 5 || weights->GetValue(0) != 0.f ||
      weights->GetValue(3) != 0.f || std::abs(weights->GetValue(1) - halfWeight) > 1e-6 ||
      std::abs(weights->GetValue(2) - primaryWeight) > 1e-6 ||
      std::abs(weights->GetValue(4) - (1. - firstWeight)) > 1e-6)
    {
      std::cerr << "Unexpected in-between weights at frame " << frame << '\n';
      return EXIT_FAILURE;
    }
    if (!::CheckPackedDeformation(mapper->GetInput()))
    {
      return EXIT_FAILURE;
    }
  }
  if (gpuSupported)
  {
    auto gpuImporter = ::CreateImporter(gpuWindow);
    gpuImporter->SetFileName(filename.c_str());
    gpuImporter->Update();
    vtkMTimeType bindTime = 0;
    for (const double frame : { 1., 12.5, 24., 36., 48. })
    {
      if (!gpuImporter->UpdateAtTimeValue(frame / 24.))
      {
        return EXIT_FAILURE;
      }
      auto* mapper = vtkPolyDataMapper::SafeDownCast(
        gpuImporter->GetRenderer()->GetActors()->GetLastActor()->GetMapper());
      vtkPolyData* polydata = mapper->GetInput();
      auto* bindPositions = polydata->GetPointData()->GetArray("blendShapeBindPositions");
      auto* weights =
        vtkFloatArray::SafeDownCast(polydata->GetFieldData()->GetArray("blendShapeWeights"));
      const double firstWeight = frame <= 24. ? (frame - 1.) / 23. : (48. - frame) / 24.;
      if (!weights || std::abs(weights->GetValue(4) - (1. - firstWeight)) > 1e-6)
      {
        std::cerr << "GPU morphing did not update weights\n";
        return EXIT_FAILURE;
      }
      if (bindTime && polydata->GetPoints()->GetMTime() != bindTime)
      {
        std::cerr << "GPU morphing modified CPU points\n";
        return EXIT_FAILURE;
      }
      bindTime = polydata->GetPoints()->GetMTime();
      for (vtkIdType i = 0; i < polydata->GetNumberOfPoints(); ++i)
      {
        double actual[3], expected[3];
        polydata->GetPoint(i, actual);
        bindPositions->GetTuple(i, expected);
        for (int component = 0; component < 3; ++component)
        {
          if (actual[component] != expected[component])
          {
            return EXIT_FAILURE;
          }
        }
      }
    }
  }
  auto subsetImporter = ::CreateImporter(cpuWindow);
  const std::string subsetFile = std::string(argv[1]) + "data/SimpleBlendShapesSubsets.usda";
  subsetImporter->SetFileName(subsetFile.c_str());
  subsetImporter->Update();
  for (const double frame : { 1., 12.5, 24., 48. })
  {
    if (!subsetImporter->UpdateAtTimeValue(frame / 24.) ||
      subsetImporter->GetRenderer()->GetActors()->GetNumberOfItems() != 2)
    {
      return EXIT_FAILURE;
    }
    vtkCollectionSimpleIterator iterator;
    auto* actors = subsetImporter->GetRenderer()->GetActors();
    actors->InitTraversal(iterator);
    const vtkFloatArray* sharedWeights = nullptr;
    while (auto* actor = actors->GetNextActor(iterator))
    {
      auto* polydata = vtkPolyDataMapper::SafeDownCast(actor->GetMapper())->GetInput();
      auto* weights =
        vtkFloatArray::SafeDownCast(polydata->GetFieldData()->GetArray("blendShapeWeights"));
      if (!::CheckPackedDeformation(polydata) || (sharedWeights && sharedWeights != weights))
      {
        std::cerr << "Subset morphing data is not shared or correctly deformed\n";
        return EXIT_FAILURE;
      }
      sharedWeights = weights;
    }
  }
  // Time-sampled base points must remain animated alongside blend shape weights.
  // Independent importers select their path once, before any animation updates.
  for (const bool render : { false, true })
  {
    const bool gpu = render && gpuSupported;
    auto animatedImporter =
      ::CreateImporter(render ? gpuWindow.GetPointer() : cpuWindow.GetPointer());
    const std::string animatedFile =
      std::string(argv[1]) + "data/SimpleBlendShapesAnimatedPoints.usda";
    animatedImporter->SetFileName(animatedFile.c_str());
    animatedImporter->Update();
    for (const double frame : { 1., 24., 48., 1. })
    {
      if (!animatedImporter->UpdateAtTimeValue(frame / 24.))
      {
        return EXIT_FAILURE;
      }
      auto* actor = animatedImporter->GetRenderer()->GetActors()->GetLastActor();
      auto* data = vtkPolyDataMapper::SafeDownCast(actor->GetMapper())->GetInput();
      const double shift = frame == 1. ? 0. : (frame == 24. ? 1. : 2.);
      const double expectedY = -0.5 + (!gpu && frame == 24. ? 0.2 : 0.);
      auto* bind =
        vtkFloatArray::SafeDownCast(data->GetPointData()->GetArray("blendShapeBindPositions"));
      if (!bind || std::abs(bind->GetComponent(0, 0) - (-0.5 + shift)) > 1e-6 ||
        std::abs(data->GetPoint(0)[0] - (-0.5 + shift)) > 1e-6 ||
        std::abs(data->GetPoint(0)[1] - expectedY) > 1e-6)
      {
        std::cerr << "Morphing froze time-sampled base points\n";
        return EXIT_FAILURE;
      }
    }
  }
  // Independent importers select their path once, before any animation updates.
  for (const bool render : { false, true })
  {
    auto skinImporter = ::CreateImporter(render ? gpuWindow.GetPointer() : cpuWindow.GetPointer());
    const std::string skinFile = std::string(argv[1]) + "data/SimpleBlendShapesAnimatedSkin.usda";
    skinImporter->SetFileName(skinFile.c_str());
    skinImporter->Update();
    for (const double frame : { 1., 24., 48., 1. })
    {
      if (!skinImporter->UpdateAtTimeValue(frame / 24.))
      {
        return EXIT_FAILURE;
      }
      vtkPolyData* data = nullptr;
      vtkCollectionSimpleIterator iterator;
      auto* actors = skinImporter->GetImportedActors();
      for (actors->InitTraversal(iterator); auto* actor = actors->GetNextActor(iterator);)
      {
        auto* mapper = vtkPolyDataMapper::SafeDownCast(actor->GetMapper());
        auto* input = mapper ? mapper->GetInput() : nullptr;
        if (input && input->GetFieldData()->GetArray("blendShapeOffsets"))
        {
          data = input;
          break;
        }
      }
      if (!data)
      {
        return EXIT_FAILURE;
      }
      auto* joints = data->GetPointData()->GetArray("JOINTS_0");
      auto* skinWeights = data->GetPointData()->GetArray("WEIGHTS_0");
      if (!joints || !skinWeights || joints->GetNumberOfTuples() != data->GetNumberOfPoints() ||
        skinWeights->GetNumberOfTuples() != data->GetNumberOfPoints() ||
        joints->GetNumberOfComponents() != 4 || skinWeights->GetNumberOfComponents() != 4)
      {
        std::cerr << "Rebuilding animated geometry discarded skinning attributes\n";
        return EXIT_FAILURE;
      }
      for (vtkIdType i = 0; i < data->GetNumberOfPoints(); ++i)
      {
        if (joints->GetComponent(i, 0) != (frame == 24. ? 1. : 0.) ||
          skinWeights->GetComponent(i, 0) != 1.)
        {
          std::cerr << "Joint influences were not sampled at the current time\n";
          return EXIT_FAILURE;
        }
      }
    }
  }
  // Hidden blend shapes, skinning-only targets and malformed offsets share a skeleton.
  auto errorImporter = ::CreateImporter(cpuWindow);
  const std::string errorFile = std::string(argv[1]) + "data/SimpleBlendShapesErrorPaths.usda";
  errorImporter->SetFileName(errorFile.c_str());
  int warningCount = 0;
  vtkNew<vtkCallbackCommand> warningObserver;
  warningObserver->SetClientData(&warningCount);
  warningObserver->SetCallback(
    [](vtkObject*, unsigned long, void* data, void*) { ++(*static_cast<int*>(data)); });
  errorImporter->AddObserver(vtkCommand::WarningEvent, warningObserver);
  if (!errorImporter->Update())
  {
    return EXIT_FAILURE;
  }
  warningCount = 0;
  if (!errorImporter->UpdateAtTimeValue(1.) || warningCount == 0)
  {
    std::cerr << "Invalid blend shape indices were not diagnosed\n";
    return EXIT_FAILURE;
  }
  vtkCollectionSimpleIterator errorIterator;
  auto* errorActors = errorImporter->GetImportedActors();
  int skinnedMeshes = 0;
  int morphingMeshes = 0;
  for (errorActors->InitTraversal(errorIterator);
       auto* actor = errorActors->GetNextActor(errorIterator);)
  {
    auto* mapper = vtkPolyDataMapper::SafeDownCast(actor->GetMapper());
    auto* data = mapper ? mapper->GetInput() : nullptr;
    if (!data || !data->GetPointData()->GetArray("WEIGHTS_0"))
    {
      continue;
    }
    ++skinnedMeshes;
    morphingMeshes += data->GetFieldData()->GetArray("blendShapeOffsets") != nullptr;
    if (data->GetNumberOfPoints() != 8 || std::abs(data->GetPoint(0)[1] + 0.5) > 1e-6)
    {
      std::cerr << "Invalid deformation modified the mesh or affected a skinning-only target\n";
      return EXIT_FAILURE;
    }
  }
  if (skinnedMeshes != 2 || morphingMeshes != 1)
  {
    std::cerr << "Hidden and skinning-only targets were not handled independently\n";
    return EXIT_FAILURE;
  }
  warningCount = 0;
  if (!errorImporter->UpdateAtTimeValue(1. / 24.) || warningCount != 0)
  {
    std::cerr << "The importer did not recover at a valid blend shape sample\n";
    return EXIT_FAILURE;
  }
  return EXIT_SUCCESS;
}

#include "vtkF3DUSDImporter.h"

#include <vtkActor.h>
#include <vtkActorCollection.h>
#include <vtkFieldData.h>
#include <vtkFloatArray.h>
#include <vtkIdTypeArray.h>
#include <vtkIntArray.h>
#include <vtkNew.h>
#include <vtkPointData.h>
#include <vtkPoints.h>
#include <vtkPolyData.h>
#include <vtkPolyDataMapper.h>
#include <vtkRenderer.h>

#include <cmath>
#include <iostream>
#include <string>

namespace
{
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
    int range[2];
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
      float offset[4];
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
  for (bool faceVarying : { false, true })
  {
    const std::string filename = std::string(argv[1]) + "data/" +
      (faceVarying ? "SimpleBlendShapesFaceVarying.usda" : "SimpleBlendShapes.usda");
    vtkNew<vtkF3DUSDImporter> importer;
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
      int range[2];
      ranges->GetTypedTuple(i, range);
      if (range[0] != source || range[1] != source + 1)
      {
        std::cerr << "Blend shape range does not follow the source vertex\n";
        return EXIT_FAILURE;
      }
      float offset[4];
      offsets->GetTypedTuple(range[0], offset);
      if (offset[0] != 0.f || std::abs(offset[1] - 0.2f) > 1e-6f || offset[2] != 0.f ||
        offset[3] != (source < 4 ? 1.f : 3.f))
      {
        std::cerr << "Unexpected packed blend shape offset\n";
        return EXIT_FAILURE;
      }
    }

    // Animation order is reversed relative to the mesh's blend shape order.
    for (double frame : { 1., 12.5, 24., 36., 48., 1. })
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
      if (!CheckPackedDeformation(polydata))
      {
        return EXIT_FAILURE;
      }
      const vtkMTimeType pointsTime = polydata->GetPoints()->GetMTime();
      const vtkMTimeType weightsTime = weights->GetMTime();
      if (!importer->UpdateAtTimeValue(frame / 24.) ||
        pointsTime != polydata->GetPoints()->GetMTime() || weightsTime != weights->GetMTime() ||
        !CheckPackedDeformation(polydata))
      {
        std::cerr << "Unchanged weights recomputed the output points\n";
        return EXIT_FAILURE;
      }
      // External edits must invalidate the cached deformation even at the same time.
      polydata->GetPoints()->SetPoint(0, 100., 100., 100.);
      polydata->GetPoints()->Modified();
      if (!importer->UpdateAtTimeValue(frame / 24.) || !CheckPackedDeformation(polydata))
      {
        return EXIT_FAILURE;
      }
    }
  }

  vtkNew<vtkF3DUSDImporter> importer;
  const std::string filename = std::string(argv[1]) + "data/SimpleBlendShapesInbetween.usda";
  importer->SetFileName(filename.c_str());
  importer->Update();
  for (double frame : { 1., 12.5, 24., 36., 48., 1. })
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
    if (!CheckPackedDeformation(mapper->GetInput()))
    {
      return EXIT_FAILURE;
    }
  }
  importer->SetUseGPUMorphing(true);
  vtkMTimeType bindTime = 0;
  for (double frame : { 1., 12.5, 24., 36., 48. })
  {
    if (!importer->UpdateAtTimeValue(frame / 24.))
    {
      return EXIT_FAILURE;
    }
    auto* mapper = vtkPolyDataMapper::SafeDownCast(
      importer->GetRenderer()->GetActors()->GetLastActor()->GetMapper());
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
  auto* gpuPolydata = vtkPolyDataMapper::SafeDownCast(
    importer->GetRenderer()->GetActors()->GetLastActor()->GetMapper())
                        ->GetInput();
  vtkNew<vtkPoints> replacement;
  replacement->DeepCopy(gpuPolydata->GetPoints());
  replacement->SetPoint(0, 100., 100., 100.);
  gpuPolydata->SetPoints(replacement);
  if (!importer->UpdateAtTimeValue(0.5))
  {
    return EXIT_FAILURE;
  }
  double restored[3], expectedBind[3];
  gpuPolydata->GetPoint(0, restored);
  gpuPolydata->GetPointData()->GetArray("blendShapeBindPositions")->GetTuple(0, expectedBind);
  for (int component = 0; component < 3; ++component)
  {
    if (restored[component] != expectedBind[component])
    {
      std::cerr << "Replacement CPU geometry was not initialized to bind pose\n";
      return EXIT_FAILURE;
    }
  }
  importer->SetUseGPUMorphing(false);
  if (!importer->UpdateAtTimeValue(12.5 / 24.) ||
    !CheckPackedDeformation(vtkPolyDataMapper::SafeDownCast(
      importer->GetRenderer()->GetActors()->GetLastActor()->GetMapper())
        ->GetInput()))
  {
    std::cerr << "CPU fallback did not restore current deformation\n";
    return EXIT_FAILURE;
  }
  vtkNew<vtkF3DUSDImporter> subsetImporter;
  const std::string subsetFile = std::string(argv[1]) + "data/SimpleBlendShapesSubsets.usda";
  subsetImporter->SetFileName(subsetFile.c_str());
  subsetImporter->Update();
  for (double frame : { 1., 12.5, 24., 48. })
  {
    if (!subsetImporter->UpdateAtTimeValue(frame / 24.) ||
      subsetImporter->GetRenderer()->GetActors()->GetNumberOfItems() != 2)
    {
      return EXIT_FAILURE;
    }
    vtkCollectionSimpleIterator iterator;
    auto* actors = subsetImporter->GetRenderer()->GetActors();
    actors->InitTraversal(iterator);
    vtkFloatArray* sharedWeights = nullptr;
    while (auto* actor = actors->GetNextActor(iterator))
    {
      auto* polydata = vtkPolyDataMapper::SafeDownCast(actor->GetMapper())->GetInput();
      auto* weights =
        vtkFloatArray::SafeDownCast(polydata->GetFieldData()->GetArray("blendShapeWeights"));
      if (!CheckPackedDeformation(polydata) || (sharedWeights && sharedWeights != weights))
      {
        std::cerr << "Subset morphing data is not shared or correctly deformed\n";
        return EXIT_FAILURE;
      }
      sharedWeights = weights;
    }
  }
  return EXIT_SUCCESS;
}

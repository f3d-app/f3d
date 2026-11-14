#include "F3DSparseMorphingTestUtilities.h"

#include "vtkF3DPolyDataMapper.h"
#include "vtkF3DSparseMorphing.h"

#include <vtkFieldData.h>
#include <vtkFloatArray.h>
#include <vtkIntArray.h>
#include <vtkOpenGLBufferObject.h>
#include <vtkOpenGLRenderWindow.h>
#include <vtkOpenGLShaderCache.h>
#include <vtkPointData.h>
#include <vtkPoints.h>
#include <vtkPolyData.h>
#include <vtkRenderWindow.h>
#include <vtkShader.h>
#include <vtkShaderProgram.h>
#include <vtk_glad.h>

#include <cmath>
#include <iostream>
#include <limits>
#include <string>

namespace
{
class MorphingMapper : public vtkF3DPolyDataMapper
{
public:
  static MorphingMapper* New()
  {
    return new MorphingMapper;
  }
  using vtkF3DPolyDataMapper::UpdateMorphingBuffers;

  bool MakeOffsetsBufferIncompatible()
  {
    return this->BlendShapeOffsets->GenerateBuffer(vtkOpenGLBufferObject::ElementArrayBuffer);
  }

  bool Check(float expected)
  {
    float offset[4], weight;
    return this->BlendShapeOffsets->Download(offset, 4) &&
      this->BlendShapeWeights->Download(&weight, 1) && offset[0] == 0.f && offset[1] == 0.2f &&
      offset[2] == 0.f && offset[3] == 0.f && weight == expected;
  }
};
}

int TestF3DPolyDataMapperMorphing(int, char*[])
{
  vtkNew<vtkRenderWindow> window;
  window->OffScreenRenderingOn();
  window->Start();
  if (!F3DTest::SupportsSparseMorphing(window))
  {
    std::cout << "Sparse morphing is unavailable; skipping morphing buffer test\n";
    return EXIT_SUCCESS;
  }

  vtkNew<MorphingMapper> mapper;
  vtkNew<vtkPolyData> polydata;
  if (mapper->UpdateMorphingBuffers(polydata))
  {
    return EXIT_FAILURE;
  }
  vtkNew<vtkFloatArray> offsets;
  offsets->SetName("blendShapeOffsets");
  offsets->SetNumberOfComponents(4);
  const float offset[4] = { 0.f, 0.2f, 0.f, 0.f };
  offsets->InsertNextTypedTuple(offset);
  polydata->GetFieldData()->AddArray(offsets);
  vtkNew<vtkFloatArray> weights;
  weights->SetName("blendShapeWeights");
  weights->InsertNextValue(0.f);
  polydata->GetFieldData()->AddArray(weights);

  // Exercise an actual VTK upload refusal without exhausting GPU memory.
  if (!mapper->MakeOffsetsBufferIncompatible() || mapper->UpdateMorphingBuffers(polydata))
  {
    std::cerr << "An incompatible OpenGL buffer was accepted for morphing\n";
    return EXIT_FAILURE;
  }
  mapper->ReleaseGraphicsResources(window);

  vtkNew<vtkPoints> point;
  point->InsertNextPoint(0., 0., 0.);
  polydata->SetPoints(point);
  vtkNew<vtkFloatArray> bindPosition;
  bindPosition->SetName("blendShapeBindPositions");
  bindPosition->SetNumberOfComponents(3);
  bindPosition->InsertNextTuple3(0., 0., 0.);
  polydata->GetPointData()->AddArray(bindPosition);
  vtkNew<vtkIntArray> vertexRange;
  vertexRange->SetName("blendShapeRanges");
  vertexRange->SetNumberOfComponents(2);
  vertexRange->InsertNextTuple2(0., 1.);
  polydata->GetPointData()->AddArray(vertexRange);
  if (!mapper->PrepareSparseMorphing(vtkOpenGLRenderWindow::SafeDownCast(window), polydata) ||
    !mapper->GetHasSparseMorphing())
  {
    return EXIT_FAILURE;
  }

  const vtkMTimeType preparedTime = mapper->GetMTime();
  if (!mapper->PrepareSparseMorphing(vtkOpenGLRenderWindow::SafeDownCast(window), polydata) ||
    mapper->GetMTime() != preparedTime)
  {
    std::cerr << "Unchanged sparse data invalidated the mapper\n";
    return EXIT_FAILURE;
  }

  // Unsupported contexts and missing geometry must disable GPU morphing safely.
  if (mapper->PrepareSparseMorphing(nullptr, polydata) || mapper->GetHasSparseMorphing() ||
    mapper->PrepareSparseMorphing(vtkOpenGLRenderWindow::SafeDownCast(window), nullptr) ||
    !mapper->PrepareSparseMorphing(vtkOpenGLRenderWindow::SafeDownCast(window), polydata))
  {
    return EXIT_FAILURE;
  }

  // An inverted interval is invalid even when both endpoints are in the buffer.
  vertexRange->SetTuple2(0, 1, 0);
  vertexRange->Modified();
  if (mapper->PrepareSparseMorphing(vtkOpenGLRenderWindow::SafeDownCast(window), polydata))
  {
    std::cerr << "Inverted sparse interval enabled GPU morphing\n";
    return EXIT_FAILURE;
  }
  vertexRange->SetTuple2(0, 0, 1);
  vertexRange->Modified();

  vtkNew<vtkIntArray> replacementRange;
  replacementRange->SetName("blendShapeRanges");
  replacementRange->SetNumberOfComponents(2);
  replacementRange->InsertNextTuple2(1, 0);
  polydata->GetPointData()->AddArray(replacementRange);
  if (mapper->PrepareSparseMorphing(vtkOpenGLRenderWindow::SafeDownCast(window), polydata))
  {
    std::cerr << "Replacing ranges retained cached validation\n";
    return EXIT_FAILURE;
  }
  polydata->GetPointData()->AddArray(vertexRange);

  for (const int invalidEnd : { -1, 2, 16777217 })
  {
    vertexRange->SetComponent(0, 1, invalidEnd);
    vertexRange->Modified();
    if (mapper->PrepareSparseMorphing(vtkOpenGLRenderWindow::SafeDownCast(window), polydata) ||
      mapper->GetHasSparseMorphing())
    {
      std::cerr << "Invalid sparse range enabled GPU morphing\n";
      return EXIT_FAILURE;
    }
  }
  vertexRange->SetComponent(0, 1, 1);
  vertexRange->Modified();
  if (!mapper->PrepareSparseMorphing(vtkOpenGLRenderWindow::SafeDownCast(window), polydata))
  {
    return EXIT_FAILURE;
  }

  // The range-validation cache must also follow changes in offset count.
  offsets->InsertNextTuple4(0., 0.1, 0., 0.);
  offsets->Modified();
  vertexRange->SetTuple2(0, 0, 2);
  vertexRange->Modified();
  if (!mapper->PrepareSparseMorphing(vtkOpenGLRenderWindow::SafeDownCast(window), polydata))
  {
    return EXIT_FAILURE;
  }
  offsets->SetNumberOfTuples(1);
  offsets->Modified();
  if (mapper->PrepareSparseMorphing(vtkOpenGLRenderWindow::SafeDownCast(window), polydata))
  {
    std::cerr << "Shrinking offsets retained cached range validation\n";
    return EXIT_FAILURE;
  }
  offsets->InsertNextTuple4(0., 0.1, 0., 0.);
  offsets->Modified();
  if (!mapper->PrepareSparseMorphing(vtkOpenGLRenderWindow::SafeDownCast(window), polydata))
  {
    return EXIT_FAILURE;
  }
  offsets->SetNumberOfTuples(1);
  offsets->Modified();
  vertexRange->SetTuple2(0, 0, 1);
  vertexRange->Modified();

  // A topology update must not enable GPU morphing until all point arrays agree.
  point->InsertNextPoint(1., 0., 0.);
  point->Modified();
  if (mapper->PrepareSparseMorphing(vtkOpenGLRenderWindow::SafeDownCast(window), polydata))
  {
    std::cerr << "Mismatched point arrays enabled GPU morphing\n";
    return EXIT_FAILURE;
  }
  bindPosition->InsertNextTuple3(1., 0., 0.);
  bindPosition->Modified();
  vertexRange->InsertNextTuple2(0, 1);
  vertexRange->Modified();
  if (!mapper->PrepareSparseMorphing(vtkOpenGLRenderWindow::SafeDownCast(window), polydata))
  {
    return EXIT_FAILURE;
  }
  point->SetNumberOfPoints(1);
  point->Modified();
  bindPosition->SetNumberOfTuples(1);
  bindPosition->Modified();
  vertexRange->SetNumberOfTuples(1);
  vertexRange->Modified();
  if (!mapper->PrepareSparseMorphing(vtkOpenGLRenderWindow::SafeDownCast(window), polydata))
  {
    return EXIT_FAILURE;
  }

  for (const float weight : { 0.f, 1.f, 0.5f, 0.f })
  {
    weights->SetValue(0, weight);
    weights->Modified();
    if (!mapper->UpdateMorphingBuffers(polydata) || !mapper->Check(weight) ||
      !mapper->UpdateMorphingBuffers(polydata) || !mapper->Check(weight))
    {
      return EXIT_FAILURE;
    }
  }
  mapper->ReleaseGraphicsResources(window);
  if (mapper->GetHasSparseMorphing())
  {
    return EXIT_FAILURE;
  }
  if (!mapper->UpdateMorphingBuffers(polydata) || !mapper->Check(0.f))
  {
    return EXIT_FAILURE;
  }
  mapper->ReleaseGraphicsResources(window);
  vtkNew<vtkFloatArray> replacement;
  replacement->SetName("blendShapeWeights");
  replacement->InsertNextValue(0.75f);
  polydata->GetFieldData()->AddArray(replacement);
  if (!mapper->UpdateMorphingBuffers(polydata) || !mapper->Check(0.75f))
  {
    return EXIT_FAILURE;
  }

  // Exercise the packed lookup on the GPU: overlapping offsets and an empty range.
  const float secondOffset[4] = { 0.4f, 0.f, 0.f, 1.f };
  offsets->InsertNextTypedTuple(secondOffset);
  offsets->Modified();
  replacement->InsertNextValue(0.5f);
  replacement->Modified();
  vtkNew<vtkOpenGLBufferObject> positions;
  const float initial[8] = { 1.f, 2.f, 3.f, 1.f, 4.f, 5.f, 6.f, 1.f };
  const int ranges[4] = { 0, 2, 2, 2 };
  vtkNew<vtkOpenGLBufferObject> rangeBuffer;
  if (!positions->Upload(initial, 8, vtkOpenGLBufferObject::ArrayBuffer) ||
    !rangeBuffer->Upload(ranges, 4, vtkOpenGLBufferObject::ArrayBuffer) ||
    !mapper->UpdateMorphingBuffers(polydata) || !positions->BindShaderStorage(3) ||
    !rangeBuffer->BindShaderStorage(4))
  {
    return EXIT_FAILURE;
  }
  vtkNew<vtkShader> shader;
  shader->SetType(vtkShader::Compute);
  shader->SetSource(std::string("#version 430\n") + vtkF3DSparseMorphing + R"glsl(
layout(local_size_x = 1) in;
layout(std430, binding = 3) buffer Positions { vec4 positions[]; };
layout(std430, binding = 4) readonly buffer Ranges { ivec2 ranges[]; };
void main()
{
  uint vertex = gl_GlobalInvocationID.x;
  positions[vertex].xyz = f3dSparseMorphPosition(positions[vertex].xyz, ranges[vertex]);
}
)glsl");
  vtkNew<vtkShaderProgram> program;
  program->SetComputeShader(shader);
  auto* context = vtkOpenGLRenderWindow::SafeDownCast(window);
  if (!context->GetShaderCache()->ReadyShaderProgram(program))
  {
    return EXIT_FAILURE;
  }
  glDispatchCompute(2, 1, 1);
  glMemoryBarrier(GL_BUFFER_UPDATE_BARRIER_BIT);
  float actual[8];
  const float expected[8] = { 1.2f, 2.15f, 3.f, 1.f, 4.f, 5.f, 6.f, 1.f };
  if (!positions->Download(actual, 8))
  {
    return EXIT_FAILURE;
  }
  for (int i = 0; i < 8; ++i)
  {
    if (std::abs(actual[i] - expected[i]) > 1e-6f)
    {
      std::cerr << "GPU sparse morphing differs at component " << i << '\n';
      return EXIT_FAILURE;
    }
  }
  context->GetShaderCache()->ReleaseCurrentShader();
  program->ReleaseGraphicsResources(window);
  positions->ReleaseGraphicsResources();
  rangeBuffer->ReleaseGraphicsResources();
  for (const float invalidIndex : { -1.f, 2.f, 0.5f, std::numeric_limits<float>::quiet_NaN() })
  {
    offsets->SetComponent(1, 3, invalidIndex);
    offsets->Modified();
    if (mapper->UpdateMorphingBuffers(polydata))
    {
      std::cerr << "Invalid subshape index accepted for GPU morphing\n";
      return EXIT_FAILURE;
    }
  }
  offsets->SetComponent(1, 3, 1.f);
  offsets->Modified();
  if (!mapper->UpdateMorphingBuffers(polydata))
  {
    return EXIT_FAILURE;
  }
  replacement->SetNumberOfValues(1);
  replacement->Modified();
  if (mapper->UpdateMorphingBuffers(polydata))
  {
    std::cerr << "Shrinking weights left an out-of-range subshape index\n";
    return EXIT_FAILURE;
  }
  mapper->ReleaseGraphicsResources(window);
  return EXIT_SUCCESS;
}

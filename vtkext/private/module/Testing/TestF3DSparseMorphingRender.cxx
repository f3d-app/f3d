#include "vtkF3DPolyDataMapper.h"
#include "vtkF3DRenderPass.h"

#include <vtkActor.h>
#include <vtkCamera.h>
#include <vtkCellArray.h>
#include <vtkFieldData.h>
#include <vtkFloatArray.h>
#include <vtkIntArray.h>
#include <vtkLight.h>
#include <vtkPointData.h>
#include <vtkPoints.h>
#include <vtkPolyData.h>
#include <vtkProperty.h>
#include <vtkRenderWindow.h>
#include <vtkRenderer.h>
#include <vtkShader.h>
#include <vtkShaderProperty.h>
#include <vtkUniforms.h>
#include <vtkUnsignedShortArray.h>
#include <vtk_glad.h>

#include <algorithm>
#include <cmath>
#include <iostream>
#include <memory>
#include <vector>

int TestF3DSparseMorphingRender(int, char*[])
{
  vtkNew<vtkRenderWindow> window;
  window->OffScreenRenderingOn();
  window->SetSize(64, 64);
  window->SetMultiSamples(0);
  window->Start();
  if (!vtkShader::IsComputeShaderSupported())
  {
    std::cout << "OpenGL 4.3 unavailable; skipping sparse rendering test\n";
    return EXIT_SUCCESS;
  }
  vtkNew<vtkPolyData> polydata;
  vtkNew<vtkPoints> points;
  points->InsertNextPoint(-0.5, -0.5, 0.);
  points->InsertNextPoint(0.5, -0.5, 0.);
  points->InsertNextPoint(0., 0.5, 0.);
  polydata->SetPoints(points);
  vtkNew<vtkCellArray> triangles;
  const vtkIdType triangle[3] = { 0, 1, 2 };
  triangles->InsertNextCell(3, triangle);
  polydata->SetPolys(triangles);
  vtkNew<vtkFloatArray> bindPositions;
  bindPositions->DeepCopy(points->GetData());
  bindPositions->SetName("blendShapeBindPositions");
  polydata->GetPointData()->AddArray(bindPositions);
  for (vtkIdType i = 0; i < 3; ++i)
  {
    double position[3];
    points->GetPoint(i, position);
    position[0] += 0.2;
    points->SetPoint(i, position);
  }
  points->Modified();
  vtkNew<vtkIntArray> ranges;
  ranges->SetName("blendShapeRanges");
  ranges->SetNumberOfComponents(2);
  for (int i = 0; i < 3; ++i)
  {
    ranges->InsertNextTuple2(0, 1);
  }
  polydata->GetPointData()->AddArray(ranges);
  vtkNew<vtkFloatArray> offsets;
  offsets->SetName("blendShapeOffsets");
  offsets->SetNumberOfComponents(4);
  offsets->InsertNextTuple4(0.2, 0., 0., 0.);
  polydata->GetFieldData()->AddArray(offsets);
  vtkNew<vtkFloatArray> weights;
  weights->SetName("blendShapeWeights");
  weights->InsertNextValue(1.f);
  polydata->GetFieldData()->AddArray(weights);
  vtkNew<vtkF3DPolyDataMapper> mapper;
  mapper->SetInputData(polydata);
  vtkNew<vtkActor> actor;
  actor->SetMapper(mapper);
  actor->GetProperty()->LightingOff();
  vtkNew<vtkRenderer> renderer;
  const vtkNew<vtkF3DRenderPass> pass;
  renderer->AddActor(actor);
  window->AddRenderer(renderer);
  renderer->GetActiveCamera()->SetPosition(0., 0., 3.);
  renderer->GetActiveCamera()->SetFocalPoint(0., 0., 0.);
  renderer->GetActiveCamera()->SetClippingRange(0.1, 10.);
  window->Render();
  const std::unique_ptr<unsigned char[]> reference(window->GetPixelData(0, 0, 63, 63, 0, 0));
  renderer->SetPass(pass);
  window->Render();
  if (!mapper->GetHasSparseMorphing())
  {
    return EXIT_FAILURE;
  }
  // Camera framing must follow the GPU deformation and invalidate on weight changes.
  for (const float weight : { 1.f, 0.5f, 1.f })
  {
    weights->SetValue(0, weight);
    weights->Modified();
    const double* bounds = mapper->GetBounds();
    if (std::abs(bounds[0] - (-0.5 + 0.2 * weight)) > 1e-6 ||
      std::abs(bounds[1] - (0.5 + 0.2 * weight)) > 1e-6)
    {
      std::cerr << "Sparse morphing bounds do not follow animation weights\n";
      return EXIT_FAILURE;
    }
  }
  for (vtkIdType i = 0; i < 3; ++i)
  {
    double position[3];
    points->GetPoint(i, position);
    position[0] -= 0.3;
    points->SetPoint(i, position);
  }
  points->Modified();
  window->Render();
  const std::unique_ptr<unsigned char[]> gpu(window->GetPixelData(0, 0, 63, 63, 0, 0));
  polydata->GetPointData()->RemoveArray("blendShapeRanges");
  window->Render();
  const std::unique_ptr<unsigned char[]> cpu(window->GetPixelData(0, 0, 63, 63, 0, 0));
  const bool valid = reference && gpu && cpu && !mapper->GetHasSparseMorphing() &&
    std::equal(reference.get(), reference.get() + 64 * 64 * 3, gpu.get()) &&
    !std::equal(reference.get(), reference.get() + 64 * 64 * 3, cpu.get());
  if (!valid)
  {
    std::cerr << "Sparse GPU rendering or CPU fallback produced an unexpected image\n";
    return EXIT_FAILURE;
  }

  // Scaling after morphing differs from scaling the bind positions first.
  polydata->GetPointData()->AddArray(ranges);
  actor->SetScale(1.5, 1., 1.);
  window->Render();
  const std::unique_ptr<unsigned char[]> scaled(window->GetPixelData(0, 0, 63, 63, 0, 0));
  actor->SetScale(1., 1., 1.);
  vtkNew<vtkFloatArray> jointWeights;
  jointWeights->SetName("WEIGHTS_0");
  jointWeights->SetNumberOfComponents(4);
  vtkNew<vtkUnsignedShortArray> joints;
  joints->SetName("JOINTS_0");
  joints->SetNumberOfComponents(4);
  for (int i = 0; i < 3; ++i)
  {
    jointWeights->InsertNextTuple4(1., 0., 0., 0.);
    joints->InsertNextTuple4(0., 0., 0., 0.);
  }
  polydata->GetPointData()->AddArray(jointWeights);
  polydata->GetPointData()->AddArray(joints);
  float jointMatrix[16] = { 1.5f, 0.f, 0.f, 0.f, 0.f, 1.f, 0.f, 0.f, 0.f, 0.f, 1.f, 0.f, 0.f, 0.f,
    0.f, 1.f };
  actor->GetShaderProperty()->GetVertexCustomUniforms()->SetUniformMatrix4x4v(
    "jointMatrices", 1, jointMatrix);
  window->Render();
  const std::unique_ptr<unsigned char[]> skinned(window->GetPixelData(0, 0, 63, 63, 0, 0));
  if (!scaled || !skinned || !mapper->GetHasSparseMorphing() ||
    !std::equal(scaled.get(), scaled.get() + 64 * 64 * 3, skinned.get()))
  {
    std::cerr << "Skinning was not applied after sparse morphing\n";
    return EXIT_FAILURE;
  }
  // More than 250 bones moves skinning to SSBO binding 0; morphing uses 1 and 2.
  std::vector<float> jointMatrices;
  for (int i = 0; i < 251; ++i)
  {
    jointMatrices.insert(jointMatrices.end(), jointMatrix, jointMatrix + 16);
  }
  actor->GetShaderProperty()->GetVertexCustomUniforms()->SetUniformMatrix4x4v(
    "jointMatrices", 251, jointMatrices.data());
  window->Render();
  const std::unique_ptr<unsigned char[]> ssboSkinned(window->GetPixelData(0, 0, 63, 63, 0, 0));
  if (!ssboSkinned || !std::equal(scaled.get(), scaled.get() + 64 * 64 * 3, ssboSkinned.get()))
  {
    std::cerr << "Sparse morphing conflicts with SSBO skinning\n";
    return EXIT_FAILURE;
  }
  actor->GetShaderProperty()->GetVertexCustomUniforms()->SetUniformMatrix4x4v(
    "jointMatrices", 1, jointMatrix);
  glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, 0);
  window->Render();
  int skinningBuffer = 0;
  glGetIntegeri_v(GL_SHADER_STORAGE_BUFFER_BINDING, 0, &skinningBuffer);
  const std::unique_ptr<unsigned char[]> uniformSkinned(window->GetPixelData(0, 0, 63, 63, 0, 0));
  if (skinningBuffer || !uniformSkinned ||
    !std::equal(scaled.get(), scaled.get() + 64 * 64 * 3, uniformSkinned.get()))
  {
    std::cerr << "Returning to uniform skinning retained stale SSBO state\n";
    return EXIT_FAILURE;
  }
  actor->GetShaderProperty()->GetVertexCustomUniforms()->RemoveUniform("jointMatrices");
  vtkNew<vtkFloatArray> normals;
  normals->SetNumberOfComponents(3);
  for (int i = 0; i < 3; ++i)
  {
    normals->InsertNextTuple3(0., 0., 1.);
    double position[3];
    bindPositions->GetTuple(i, position);
    position[0] += 0.2;
    points->SetPoint(i, position);
  }
  points->Modified();
  polydata->GetPointData()->SetNormals(normals);
  actor->GetProperty()->LightingOn();
  vtkNew<vtkLight> light;
  light->SetLightTypeToSceneLight();
  light->SetPosition(1., 1., 3.);
  light->SetFocalPoint(0., 0., 0.);
  renderer->RemoveAllLights();
  renderer->AddLight(light);
  polydata->GetPointData()->RemoveArray("blendShapeRanges");
  window->Render();
  const std::unique_ptr<unsigned char[]> litCPU(window->GetPixelData(0, 0, 63, 63, 0, 0));
  polydata->GetPointData()->AddArray(ranges);
  window->Render();
  const std::unique_ptr<unsigned char[]> litGPU(window->GetPixelData(0, 0, 63, 63, 0, 0));
  if (!litCPU || !litGPU || !mapper->GetHasSparseMorphing() ||
    !std::equal(litCPU.get(), litCPU.get() + 64 * 64 * 3, litGPU.get()))
  {
    std::cerr << "Lit sparse morphing differs from CPU-deformed geometry\n";
    return EXIT_FAILURE;
  }
  // Without authored normals, lighting derives normals from view-space positions.
  // A fresh pass must use morphed positions even with an automatically created light.
  polydata->GetPointData()->SetNormals(nullptr);
  renderer->RemoveAllLights();
  renderer->ReleaseGraphicsResources(window);
  renderer->SetPass(nullptr);
  offsets->SetTuple4(0, 0., 0., 0.2, 0.);
  offsets->Modified();
  for (vtkIdType i = 0; i < 3; ++i)
  {
    ranges->SetTuple2(i, 0., i == 0 ? 1. : 0.);
    double position[3];
    bindPositions->GetTuple(i, position);
    if (i == 0)
    {
      position[2] += 0.2;
    }
    points->SetPoint(i, position);
  }
  ranges->Modified();
  points->Modified();
  window->Render();
  const std::unique_ptr<unsigned char[]> flatCPU(window->GetPixelData(0, 0, 63, 63, 0, 0));
  const vtkNew<vtkF3DRenderPass> flatPass;
  renderer->SetPass(flatPass);
  window->Render();
  const std::unique_ptr<unsigned char[]> flatGPU(window->GetPixelData(0, 0, 63, 63, 0, 0));
  if (!flatCPU || !flatGPU ||
    !std::equal(flatCPU.get(), flatCPU.get() + 64 * 64 * 3, flatGPU.get()))
  {
    std::cerr << "Derived normals did not use morphed view-space positions\n";
    return EXIT_FAILURE;
  }
  window->Finalize();
  window->Render();
  const std::unique_ptr<unsigned char[]> restored(window->GetPixelData(0, 0, 63, 63, 0, 0));
  if (!restored || !mapper->GetHasSparseMorphing() ||
    !std::equal(flatGPU.get(), flatGPU.get() + 64 * 64 * 3, restored.get()))
  {
    std::cerr << "Sparse morphing did not recover after context recreation\n";
    return EXIT_FAILURE;
  }
  return EXIT_SUCCESS;
}

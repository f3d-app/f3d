#include "vtkF3DPolyDataMapper.h"
#include "vtkF3DRenderPass.h"

#include <vtkActor.h>
#include <vtkCamera.h>
#include <vtkImageData.h>
#include <vtkLookupTable.h>
#include <vtkNew.h>
#include <vtkOSPRayRendererNode.h>
#include <vtkPlaneSource.h>
#include <vtkPointData.h>
#include <vtkPolyData.h>
#include <vtkProperty.h>
#include <vtkRenderWindow.h>
#include <vtkRenderer.h>
#include <vtkShaderProperty.h>
#include <vtkUniforms.h>
#include <vtkUnsignedCharArray.h>
#include <vtkWindowToImageFilter.h>

#include <cmath>
#include <cstdlib>
#include <iostream>

int TestF3DRaytracingColorSpace(int, char*[])
{
  vtkNew<vtkPlaneSource> plane;
  plane->SetOrigin(-0.9, -0.9, 0.0);
  plane->SetPoint1(0.9, -0.9, 0.0);
  plane->SetPoint2(-0.9, 0.9, 0.0);
  plane->Update();

  const unsigned char rgb[3] = { 128, 64, 192 };
  vtkNew<vtkUnsignedCharArray> colors;
  colors->SetNumberOfComponents(3);
  for (vtkIdType i = 0; i < plane->GetOutput()->GetNumberOfPoints(); ++i)
  {
    colors->InsertNextTypedTuple(rgb);
  }
  plane->GetOutput()->GetPointData()->SetScalars(colors);

  vtkNew<vtkF3DPolyDataMapper> scalarMapper;
  scalarMapper->SetInputData(plane->GetOutput());
  scalarMapper->SetColorModeToDirectScalars();
  vtkNew<vtkF3DPolyDataMapper> solidMapper;
  solidMapper->SetInputData(plane->GetOutput());
  solidMapper->ScalarVisibilityOff();

  vtkNew<vtkActor> scalarActor;
  scalarActor->SetMapper(scalarMapper);
  scalarActor->SetPosition(-1.0, 0.0, 0.0);
  vtkNew<vtkActor> solidActor;
  solidActor->SetMapper(solidMapper);
  solidActor->SetPosition(1.0, 0.0, 0.0);
  for (vtkActor* actor : { scalarActor.GetPointer(), solidActor.GetPointer() })
  {
    actor->GetProperty()->SetInterpolationToPBR();
    actor->GetProperty()->SetRoughness(1.0);
    actor->GetProperty()->SetBaseIOR(1.0);
    actor->GetShaderProperty()->GetFragmentCustomUniforms()->SetUniformf("IBLIntensity", 1.0f);
  }
  solidActor->GetProperty()->SetColor(
    std::pow(rgb[0] / 255.0, 2.2), std::pow(rgb[1] / 255.0, 2.2), std::pow(rgb[2] / 255.0, 2.2));

  vtkNew<vtkRenderer> renderer;
  renderer->AddActor(scalarActor);
  renderer->AddActor(solidActor);
  renderer->GetActiveCamera()->SetPosition(0.0, 0.0, 4.0);
  renderer->GetActiveCamera()->SetFocalPoint(0.0, 0.0, 0.0);
  renderer->GetActiveCamera()->ParallelProjectionOn();
  renderer->GetActiveCamera()->SetParallelScale(1.0);
  renderer->ResetCameraClippingRange();

  vtkOSPRayRendererNode::SetRendererType("pathtracer", renderer);
  vtkOSPRayRendererNode::SetSamplesPerPixel(32, renderer);
  vtkOSPRayRendererNode::SetEnableDenoiser(0, renderer);
  vtkOSPRayRendererNode::SetCompositeOnGL(0, renderer);

  vtkNew<vtkF3DRenderPass> pass;
  renderer->SetPass(pass);
  vtkNew<vtkRenderWindow> window;
  window->SetSize(160, 80);
  window->SetMultiSamples(0);
  window->OffScreenRenderingOn();
  window->AddRenderer(renderer);

  vtkNew<vtkWindowToImageFilter> capture;
  capture->SetInput(window);
  capture->ReadFrontBufferOff();
  capture->ShouldRerenderOff();

  vtkNew<vtkLookupTable> lut;
  lut->SetNumberOfTableValues(2);
  lut->Build();
  for (vtkIdType i = 0; i < 2; ++i)
  {
    lut->SetTableValue(i, rgb[0] / 255.0, rgb[1] / 255.0, rgb[2] / 255.0);
  }
  scalarMapper->SetLookupTable(lut);

  for (bool texture : { false, true })
  {
    scalarMapper->SetScalarModeToUsePointData();
    scalarMapper->SetColorMode(
      texture ? VTK_COLOR_MODE_MAP_SCALARS : VTK_COLOR_MODE_DIRECT_SCALARS);
    scalarMapper->SetInterpolateScalarsBeforeMapping(texture);
    for (bool raytracing : { false, true, false, true })
    {
      pass->SetUseRaytracing(raytracing);
      window->Render();
      capture->Modified();
      capture->Update();
      vtkImageData* image = capture->GetOutput();
      for (int component = 0; component < 3; ++component)
      {
        double scalar = image->GetScalarComponentAsDouble(40, 40, 0, component);
        double solid = image->GetScalarComponentAsDouble(120, 40, 0, component);
        if (solid < 10.0 || std::abs(scalar - solid) > 3.0)
        {
          std::cerr << "Scalar/material color mismatch: texture=" << texture
                    << ", raytracing=" << raytracing << ", component=" << component << ", "
                    << scalar << " != " << solid << "\n";
          return EXIT_FAILURE;
        }
      }
    }
  }
  return EXIT_SUCCESS;
}

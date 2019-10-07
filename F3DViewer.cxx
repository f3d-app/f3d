#include "F3DViewer.h"

#include "F3DOptions.h"

#include "vtkF3DGenericImporter.h"
#include "vtkF3DOpenGLGridMapper.h"

#include <vtkActor.h>
#include <vtkActor2DCollection.h>
#include <vtkAxesActor.h>
#include <vtkBoundingBox.h>
#include <vtkCameraPass.h>
#include <vtkImageDifference.h>
#include <vtkLightsPass.h>
#include <vtkOpaquePass.h>
#include <vtkOpenGLRenderer.h>
#include <vtkOverlayPass.h>
#include <vtkPNGReader.h>
#include <vtkPNGWriter.h>
#include <vtkPointSource.h>
#include <vtkProperty.h>
#include <vtkRenderPassCollection.h>
#include <vtkRenderWindow.h>
#include <vtkRenderWindowInteractor.h>
#include <vtkRenderer.h>
#include <vtkRendererCollection.h>
#include <vtkSequencePass.h>
#include <vtkTranslucentPass.h>
#include <vtkVolumetricPass.h>
#include <vtkWindowToImageFilter.h>

#if VTK_VERSION_MAJOR == 8 && VTK_VERSION_MINOR > 2
#include <vtkDualDepthPeelingPass.h>
#include <vtkOpenGLFXAAPass.h>
#include <vtkSSAOPass.h>
#endif

//----------------------------------------------------------------------------
F3DViewer::F3DViewer(F3DOptions* options, vtkF3DGenericImporter* importer)
{
  this->Options = options;
  this->Importer = importer;

  this->RenderWindow = importer->GetRenderWindow();

  this->Renderer = this->RenderWindow->GetRenderers()->GetFirstRenderer();

  this->SetupRenderPasses();

  this->Renderer->SetBackground(this->Options->BackgroundColor.data());

  this->RenderWindowInteractor->SetRenderWindow(this->RenderWindow);
  this->RenderWindowInteractor->SetInteractorStyle(this->InteractorStyle);

  this->InteractorStyle->SetViewer(this);

  this->RenderWindow->SetSize(this->Options->WindowSize[0], this->Options->WindowSize[1]);
  this->RenderWindow->SetWindowName(f3d::AppTitle.c_str());

  vtkNew<vtkAxesActor> axes;
  this->AxisWidget->SetOrientationMarker(axes);
  this->AxisWidget->SetInteractor(this->RenderWindowInteractor);
  this->AxisWidget->SetViewport(0.85, 0.0, 1.0, 0.15);

  vtkActor2DCollection* actors = this->Renderer->GetActors2D();
  actors->InitTraversal();
  vtkActor2D* currentActor;
  while ((currentActor = actors->GetNextActor2D()) != nullptr)
  {
    if (currentActor->IsA("vtkScalarBarActor"))
    {
      this->ScalarBarActor = currentActor;
      break;
    }
  }
}

//----------------------------------------------------------------------------
void F3DViewer::ShowAxis(bool show)
{
  this->AxisWidget->SetEnabled(show);
  if (show)
  {
    this->AxisWidget->InteractiveOff();
  }
}

//----------------------------------------------------------------------------
void F3DViewer::SetupRenderPasses()
{
#if VTK_VERSION_MAJOR == 8 && VTK_VERSION_MINOR > 2
  vtkOpenGLRenderer* renderer = vtkOpenGLRenderer::SafeDownCast(this->Renderer);

  vtkNew<vtkLightsPass> lightsP;
  vtkNew<vtkOpaquePass> opaqueP;
  vtkNew<vtkTranslucentPass> translucentP;
  vtkNew<vtkVolumetricPass> volumeP;
  vtkNew<vtkOverlayPass> overlayP;

  vtkNew<vtkRenderPassCollection> collection;
  collection->AddItem(lightsP);

  // opaque passes
  if (this->Options->SSAO)
  {
    double bounds[6];
    this->Renderer->ComputeVisiblePropBounds(bounds);

    vtkBoundingBox bbox(bounds);

    vtkNew<vtkCameraPass> ssaoCamP;
    ssaoCamP->SetDelegatePass(opaqueP);

    vtkNew<vtkSSAOPass> ssaoP;
    ssaoP->SetRadius(0.1 * bbox.GetDiagonalLength());
    ssaoP->SetBias(0.001 * bbox.GetDiagonalLength());
    ssaoP->SetKernelSize(200);
    ssaoP->SetDelegatePass(ssaoCamP);

    collection->AddItem(ssaoP);
  }
  else
  {
    collection->AddItem(opaqueP);
  }

  // translucent and volumic passes
  if (this->Options->DepthPeeling)
  {
    vtkNew<vtkDualDepthPeelingPass> ddpP;
    ddpP->SetTranslucentPass(translucentP);
    ddpP->SetVolumetricPass(volumeP);
    collection->AddItem(ddpP);
  }
  else
  {
    collection->AddItem(translucentP);
    collection->AddItem(volumeP);
  }

  collection->AddItem(overlayP);

  vtkNew<vtkSequencePass> sequence;
  sequence->SetPasses(collection);

  vtkNew<vtkCameraPass> cameraP;
  cameraP->SetDelegatePass(sequence);

  if (this->Options->FXAA)
  {
    vtkNew<vtkOpenGLFXAAPass> fxaaP;
    fxaaP->SetDelegatePass(cameraP);

    renderer->SetPass(fxaaP);
  }
  else
  {
    renderer->SetPass(cameraP);
  }
#else
  this->Renderer->SetUseFXAA(this->Options->FXAA);
  this->Renderer->SetUseDepthPeeling(this->Options->DepthPeeling);
#endif
}

//----------------------------------------------------------------------------
bool F3DViewer::IsAxisVisible()
{
  return this->AxisWidget->GetEnabled();
}

//----------------------------------------------------------------------------
void F3DViewer::ShowGrid(bool show)
{
  this->Renderer->SetClippingRangeExpansion(0.99);
  this->Renderer->ResetCameraClippingRange();

  double bounds[6];
  this->Renderer->ComputeVisiblePropBounds(bounds);

  vtkBoundingBox bbox(bounds);

  vtkNew<vtkPointSource> gridPointSource;
  gridPointSource->SetNumberOfPoints(1);
  gridPointSource->SetRadius(0);
  gridPointSource->SetCenter(0, bounds[2], 0);

  vtkNew<vtkF3DOpenGLGridMapper> gridMapper;
  gridMapper->SetInputConnection(gridPointSource->GetOutputPort());
  gridMapper->SetFadeDistance(bbox.GetDiagonalLength());

  this->GridActor->GetProperty()->SetColor(0.0, 0.0, 0.0);
  this->GridActor->ForceTranslucentOn();
  this->GridActor->SetMapper(gridMapper);

  this->Renderer->RemoveActor(this->GridActor);
  this->Renderer->AddActor(this->GridActor);

  this->GridActor->SetVisibility(show);
}

//----------------------------------------------------------------------------
bool F3DViewer::IsGridVisible()
{
  return this->GridActor->GetVisibility();
}

//----------------------------------------------------------------------------
void F3DViewer::ShowScalarBar(bool show)
{
  if (this->ScalarBarActor)
  {
    this->ScalarBarActor->SetVisibility(show);
  }
}

//----------------------------------------------------------------------------
bool F3DViewer::IsScalarBarVisible()
{
  return this->ScalarBarActor && this->ScalarBarActor->GetVisibility();
}

//----------------------------------------------------------------------------
void F3DViewer::SetupWithOptions()
{
  this->ShowGrid(this->Options->Grid);
  this->ShowAxis(this->Options->Axis);
}

//----------------------------------------------------------------------------
int F3DViewer::Start()
{
  this->SetupWithOptions();

  if (!this->Options->Output.empty() || !this->Options->Reference.empty())
  {
    vtkNew<vtkWindowToImageFilter> rtW2if;
    rtW2if->SetInput(this->RenderWindow);

    this->RenderWindow->OffScreenRenderingOn();
    this->Render();

    if (!this->Options->Output.empty())
    {
      vtkNew<vtkPNGWriter> writer;
      writer->SetInputConnection(rtW2if->GetOutputPort());
      writer->SetFileName(this->Options->Output.c_str());
      writer->Write();

      return EXIT_SUCCESS;
    }

    vtkNew<vtkPNGReader> reader;
    reader->SetFileName(this->Options->Reference.c_str());

    vtkNew<vtkImageDifference> diff;
    diff->SetInputConnection(rtW2if->GetOutputPort());
    diff->SetImageConnection(reader->GetOutputPort());
    diff->Update();

    double error = diff->GetThresholdedError();
    cout << "Diff threshold error = " << error << endl;
    if (error > this->Options->RefThreshold)
    {
      return EXIT_FAILURE;
    }
    return EXIT_SUCCESS;
  }

  this->Render();
  this->RenderWindowInteractor->Start();
  return EXIT_SUCCESS;
}

//----------------------------------------------------------------------------
void F3DViewer::Render()
{
  this->RenderWindow->Render();
}

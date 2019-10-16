#include "F3DViewer.h"

#include "F3DOptions.h"

#include "vtkF3DOpenGLGridMapper.h"

#include <vtkActor.h>
#include <vtkActor2DCollection.h>
#include <vtkAxesActor.h>
#include <vtkBoundingBox.h>
#include <vtkCallbackCommand.h>
#include <vtkCameraPass.h>
#include <vtkImageDifference.h>
#include <vtkImporter.h>
#include <vtkLightsPass.h>
#include <vtkOpaquePass.h>
#include <vtkOpenGLRenderer.h>
#include <vtkOverlayPass.h>
#include <vtkPNGReader.h>
#include <vtkPNGWriter.h>
#include <vtkProperty.h>
#include <vtkRenderPassCollection.h>
#include <vtkRenderWindowInteractor.h>
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
F3DViewer::F3DViewer(F3DOptions* options, vtkImporter* importer)
{
  this->Options = options;
  this->Importer = importer;

  this->RenderWindow->SetOffScreenRendering(
    !this->Options->Output.empty() || !this->Options->Reference.empty());

  this->Importer->SetRenderWindow(this->RenderWindow);
  this->RenderWindow->AddRenderer(this->Renderer);

  this->Renderer->SetBackground(this->Options->BackgroundColor.data());

  this->RenderWindowInteractor->SetRenderWindow(this->RenderWindow);
  this->RenderWindowInteractor->SetInteractorStyle(this->InteractorStyle);
  this->RenderWindowInteractor->Initialize();

  this->InteractorStyle->SetViewer(this);

  this->RenderWindow->SetSize(this->Options->WindowSize[0], this->Options->WindowSize[1]);
  this->RenderWindow->SetWindowName(f3d::AppTitle.c_str());

  if (!this->Options->HideProgress)
  {
    vtkNew<vtkCallbackCommand> progressCallback;
    progressCallback->SetClientData(this);
    progressCallback->SetCallback(
      [](vtkObject*, unsigned long, void* clientData, void* callData) {
        static_cast<F3DViewer*>(clientData)->SetProgress(*static_cast<double*>(callData));
      });
    this->Importer->AddObserver(vtkCommand::ProgressEvent, progressCallback);

    this->ProgressWidget->SetInteractor(this->RenderWindowInteractor);
    this->ProgressWidget->SetRepresentation(this->ProgressRepresentation);

    this->ProgressRepresentation->SetProgressRate(0.0);
    this->ProgressRepresentation->SetPosition(0.25, 0.45);
    this->ProgressRepresentation->SetProgressBarColor(1, 1, 1);
    this->ProgressRepresentation->SetBackgroundColor(1, 1, 1);
    this->ProgressRepresentation->DrawBackgroundOff();

    this->ProgressWidget->On();
  }

  // display window
  this->Render();

  // Read file
  this->Importer->Update();

  this->ProgressWidget->Off();
  this->Renderer->ResetCamera();
  this->SetupRenderPasses();
  this->Render();

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

    if (bbox.IsValid())
    {
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

  if (bbox.IsValid())
  {
    double diag = bbox.GetDiagonalLength();
    double unitSquare = pow(10.0, round(log10(diag * 0.1)));

    if (this->Options->Verbose)
    {
      cout << "Using grid unit square size = " << unitSquare << endl;
    }

    vtkNew<vtkF3DOpenGLGridMapper> gridMapper;
    gridMapper->SetFadeDistance(diag);
    gridMapper->SetUnitSquare(unitSquare);

    this->GridActor->GetProperty()->SetColor(0.0, 0.0, 0.0);
    this->GridActor->ForceTranslucentOn();
    this->GridActor->SetPosition(0, bounds[2], 0);
    this->GridActor->SetMapper(gridMapper);

    this->Renderer->RemoveActor(this->GridActor);
    this->Renderer->AddActor(this->GridActor);
  }
  else
  {
    show = false;
  }
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

//----------------------------------------------------------------------------
void F3DViewer::SetProgress(double progress)
{
  this->ProgressRepresentation->SetProgressRate(progress);
  this->Render();
}

#include "animationManager.h"

#include "interactor_impl.h"
#include "log.h"
#include "options.h"
#include "window.h"

#include <vtkDoubleArray.h>
#include <vtkImporter.h>
#include <vtkProgressBarRepresentation.h>
#include <vtkVersion.h>

#include <functional>

namespace f3d::detail
{
//----------------------------------------------------------------------------
void animationManager::Initialize(
  const options* options, interactor_impl* interactor, window* window, vtkImporter* importer)
{
  this->HasAnimation = false;

  this->Options = options;
  this->Interactor = interactor;
  this->Window = window;
  this->Importer = importer;

  // This can be -1 if animation support is not implemented in the importer
  int availAnimations = this->Importer->GetNumberOfAnimations();

  if (availAnimations > 0)
  {
    this->ProgressWidget = vtkSmartPointer<vtkProgressBarWidget>::New();
    interactor->SetInteractorOn(this->ProgressWidget);

    vtkProgressBarRepresentation* progressRep =
      vtkProgressBarRepresentation::SafeDownCast(this->ProgressWidget->GetRepresentation());
    progressRep->SetProgressRate(0.0);
    progressRep->ProportionalResizeOff();
    progressRep->SetPosition(0.0, 0.0);
    progressRep->SetPosition2(1.0, 0.0);
    progressRep->SetMinimumSize(0, 5);
    progressRep->SetProgressBarColor(1, 0, 0);
    progressRep->DrawBackgroundOff();
    progressRep->DragableOff();
    progressRep->SetShowBorderToOff();

// Complete vtkProgressBarRepresentation needs
// https://gitlab.kitware.com/vtk/vtk/-/merge_requests/7359
#if VTK_VERSION_NUMBER >= VTK_VERSION_CHECK(9, 0, 20201027)
    progressRep->DrawFrameOff();
    progressRep->SetPadding(0.0, 0.0);
#endif

    this->ProgressWidget->On();
  }
  else
  {
    this->ProgressWidget = nullptr;
  }

  if (availAnimations <= 0)
  {
    log::debug("No animations available in this file");
  }
  else
  {
    log::debug("Animation(s) available in this file are:");
  }
  for (int i = 0; i < availAnimations; i++)
  {
    log::debug(i, ": ", this->Importer->GetAnimationName(i));
  }
  log::debug("");

  int animationIndex = options->getAsInt("scene.animation.index");
  if (animationIndex != 0 && availAnimations <= 0)
  {
    log::warn("An animation index has been specified but there are no animation available.");
  }
  else if (animationIndex > 0 && animationIndex >= availAnimations)
  {
    log::warn(
      "Specified animation index is greater than the highest possible animation index, enabling "
      "the first animation.");

    this->Importer->EnableAnimation(0);
  }
  else if (animationIndex <= -1)
  {
    for (int i = 0; i < availAnimations; i++)
    {
      this->Importer->EnableAnimation(i);
    }
  }
  else
  {
    this->Importer->EnableAnimation(animationIndex);
  }

  this->TimeSteps.clear();
  this->TimeRange[0] = 0.0;
  this->TimeRange[1] = 0.0;

  vtkIdType nbAnims = this->Importer->GetNumberOfAnimations();
  for (vtkIdType animIndex = 0; animIndex < nbAnims; animIndex++)
  {
    if (this->Importer->IsAnimationEnabled(animIndex))
    {
      int nbTimeSteps;
      vtkNew<vtkDoubleArray> timeSteps;

// Complete GetTemporalInformation needs https://gitlab.kitware.com/vtk/vtk/-/merge_requests/7246
#if VTK_VERSION_NUMBER >= VTK_VERSION_CHECK(9, 0, 20201016)
      this->Importer->GetTemporalInformation(
        animIndex, this->FrameRate, nbTimeSteps, this->TimeRange, timeSteps);
#else
      this->Importer->GetTemporalInformation(animIndex, nbTimeSteps, this->TimeRange, timeSteps);
#endif

      for (vtkIdType i = 0; i < timeSteps->GetNumberOfTuples(); i++)
      {
        this->TimeSteps.insert(timeSteps->GetValue(i));
      }
    }
  }

  if (this->TimeSteps.size() > 0)
  {
    this->CurrentTimeStep = std::begin(this->TimeSteps);
    this->HasAnimation = true;
  }
  this->Playing = false;
}

//----------------------------------------------------------------------------
void animationManager::StartAnimation()
{
  if (!this->IsPlaying())
  {
    this->ToggleAnimation();
  }
}

//----------------------------------------------------------------------------
void animationManager::StopAnimation()
{
  if (this->IsPlaying())
  {
    this->ToggleAnimation();
  }
  this->ProgressWidget = nullptr;
}

//----------------------------------------------------------------------------
void animationManager::ToggleAnimation()
{
  if (this->HasAnimation)
  {
    this->Playing = !this->Playing;

    if (this->CallBackId != 0)
    {
      this->Interactor->removeTimerCallBack(this->CallBackId);
    }

    if (this->Playing)
    {
      this->CallBackId =
        this->Interactor->createTimerCallBack(1000.0 / this->FrameRate, [this]() { this->Tick(); });
      if (this->Options->getAsInt("scene.camera.index") >= 0)
      {
        this->Interactor->disableCameraMovement();
      }
      else
      {
        this->Interactor->enableCameraMovement();
      }
    }
  }
}

//----------------------------------------------------------------------------
void animationManager::Tick()
{
  if (this->HasAnimation)
  {
    vtkProgressBarRepresentation* progressRep =
      vtkProgressBarRepresentation::SafeDownCast(this->ProgressWidget->GetRepresentation());
    progressRep->SetProgressRate(std::distance(std::begin(this->TimeSteps), this->CurrentTimeStep) /
      static_cast<double>(this->TimeSteps.size() - 1));

    this->Importer->UpdateTimeStep(*this->CurrentTimeStep);
    this->Window->render();

    ++this->CurrentTimeStep;

    // repeat
    if (this->CurrentTimeStep == std::end(this->TimeSteps))
    {
      this->CurrentTimeStep = std::begin(this->TimeSteps);
    }
  }
}
}

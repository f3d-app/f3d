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
  const options* options, window* window, interactor_impl* interactor, vtkImporter* importer)
{
  this->HasAnimation = false;

  this->Options = options;
  this->Interactor = interactor;
  this->Window = window;
  this->Importer = importer;

  // This can be -1 if animation support is not implemented in the importer
  vtkIdType availAnimations = this->Importer->GetNumberOfAnimations();

  if (availAnimations > 0 && interactor)
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

  // Recover time ranges for all enabled animations
  this->TimeRange[0] = std::numeric_limits<double>::infinity();
  this->TimeRange[1] = -std::numeric_limits<double>::infinity();
  vtkIdType nbAnims = this->Importer->GetNumberOfAnimations();
  for (vtkIdType animIndex = 0; animIndex < nbAnims; animIndex++)
  {
    if (this->Importer->IsAnimationEnabled(animIndex))
    {
      double timeRange[2];
      int nbTimeSteps;
      vtkNew<vtkDoubleArray> timeSteps;

// Complete GetTemporalInformation needs https://gitlab.kitware.com/vtk/vtk/-/merge_requests/7246
#if VTK_VERSION_NUMBER >= VTK_VERSION_CHECK(9, 0, 20201016)
      // Discard timesteps, F3D only cares about real elapsed time using time range
      // Specifying the frame rate in the next call is not needed after VTK 9.2.20230603 :
      // VTK_VERSION_CHECK(9, 2, 20230603)
      double frameRate = this->Options->getAsDouble("scene.animation.frame-rate");
      this->Importer->GetTemporalInformation(
        animIndex, frameRate, nbTimeSteps, timeRange, timeSteps);
#else
      this->Importer->GetTemporalInformation(animIndex, nbTimeSteps, timeRange, timeSteps);
#endif
      // Accumulate time ranges
      this->TimeRange[0] = std::min(timeRange[0], this->TimeRange[0]);
      this->TimeRange[1] = std::max(timeRange[1], this->TimeRange[1]);
      this->HasAnimation = true;
    }
  }
  log::debug("Animation(s) time range is: [", this->TimeRange[0], ", ", this->TimeRange[1], "].");
  this->Playing = false;
  this->CurrentTime = 0;
  this->CurrentTimeSet = false;
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
  if (this->HasAnimation && this->Interactor)
  {
    this->Playing = !this->Playing;

    if (this->CallBackId != 0)
    {
      this->Interactor->removeTimerCallBack(this->CallBackId);
    }
    if (this->Playing)
    {
      // Initialize time if not already
      if (!this->CurrentTimeSet)
      {
        this->CurrentTime = this->TimeRange[0];
        this->CurrentTimeSet = true;
      }

      // Always reset previous tick when starting the animation
      this->PreviousTick = std::chrono::steady_clock::now();

      double frameRate = this->Options->getAsDouble("scene.animation.frame-rate");
      this->CallBackId =
        this->Interactor->createTimerCallBack(1000.0 / frameRate, [this]() { this->Tick(); });
    }

    if (this->Playing && this->Options->getAsInt("scene.camera.index") >= 0)
    {
      this->Interactor->disableCameraMovement();
    }
    else
    {
      this->Interactor->enableCameraMovement();
    }
  }
}

//----------------------------------------------------------------------------
void animationManager::Tick()
{
  assert(this->Interactor);

  // Compute time since previous tick
  std::chrono::steady_clock::time_point tick = std::chrono::steady_clock::now();
  auto timeInMS =
    std::chrono::duration_cast<std::chrono::milliseconds>(tick - this->PreviousTick).count();
  this->PreviousTick = tick;

  // Convert to a usable time in seconds
  double elapsedTime = static_cast<double>(timeInMS) / 1000.0;
  double animationSpeedFactor = this->Options->getAsDouble("scene.animation.speed-factor");

  // elapsedTime can be negative
  elapsedTime *= animationSpeedFactor;

  // Modulo computation, compute CurrentTime + elapsedTime in the time range.
  double delta = this->TimeRange[1] - this->TimeRange[0];
  this->CurrentTime = fmod(this->CurrentTime - this->TimeRange[0] + fmod(elapsedTime, delta) + delta, delta) + this->TimeRange[0];

  this->LoadAtTime(this->CurrentTime);
  this->Window->render();
}

//----------------------------------------------------------------------------
bool animationManager::LoadAtTime(double timeValue)
{
  if (!this->HasAnimation)
  {
    log::warn("No animation available, cannot load a specific animation time");
    return false;
  }
  if (timeValue < this->TimeRange[0] || timeValue > this->TimeRange[1])
  {
    log::warn("Provided time value: ", timeValue, " is outside of animation time range: [",
      this->TimeRange[0], ", ", this->TimeRange[1], "] .");
    return false;
  }

  this->CurrentTime = timeValue;
  this->CurrentTimeSet = true;
  this->Importer->UpdateTimeStep(this->CurrentTime);

  if (this->Interactor)
  {
    // Set progress bar
    vtkProgressBarRepresentation* progressRep =
      vtkProgressBarRepresentation::SafeDownCast(this->ProgressWidget->GetRepresentation());
    progressRep->SetProgressRate(
      (this->CurrentTime - this->TimeRange[0]) / (this->TimeRange[1] - this->TimeRange[0]));

    this->Interactor->UpdateRendererAfterInteraction();
  }
  return true;
}
}

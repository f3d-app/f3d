#include "animationManager.h"

#include "interactor_impl.h"
#include "log.h"
#include "options.h"
#include "window_impl.h"

#include "vtkF3DRenderer.h"

#include <vtkDoubleArray.h>
#include <vtkImporter.h>
#include <vtkProgressBarRepresentation.h>
#include <vtkRenderWindow.h>
#include <vtkRendererCollection.h>
#include <vtkVersion.h>

#include <cmath>
#include <functional>

namespace f3d::detail
{
//----------------------------------------------------------------------------
animationManager::animationManager(const options& options, window_impl& window)
  : Options(options)
  , Window(window)
{
  if (vtksys::SystemTools::HasEnv("CTEST_F3D_PREVENT_SKIP_FRAME"))
  {
    this->SkipFrame = false;
  }
}

//----------------------------------------------------------------------------
void animationManager::SetImporter(vtkImporter* importer)
{
  this->Importer = importer;
}

//----------------------------------------------------------------------------
void animationManager::SetInteractor(interactor_impl* interactor)
{
  this->Interactor = interactor;
}

//----------------------------------------------------------------------------
void animationManager::SetDeltaTime(double deltaTime)
{
  this->DeltaTime = deltaTime;
}

//----------------------------------------------------------------------------
void animationManager::Initialize()
{
  assert(this->Importer);
  this->HasAnimation = false;
  this->Playing = false;
  this->CurrentTime = 0;
  this->CurrentTimeSet = false;

  this->AvailAnimations = this->Importer->GetNumberOfAnimations();
  if (this->AvailAnimations > 0 && this->Interactor)
  {
    this->ProgressWidget = vtkSmartPointer<vtkProgressBarWidget>::New();
    this->Interactor->SetInteractorOn(this->ProgressWidget);

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
    progressRep->DrawFrameOff();
    progressRep->SetPadding(0.0, 0.0);
    progressRep->SetVisibility(this->Options.ui.animation_progress);
    this->ProgressWidget->On();
  }
  else
  {
    this->ProgressWidget = nullptr;
  }

  if (this->AvailAnimations <= 0)
  {
    log::debug("No animation available");
    if (this->Options.scene.animation.index > 0)
    {
      log::warn("An animation index has been specified but there are no animation available.");
    }
    return;
  }
  else
  {
    log::debug("Animation(s) available are:");
  }
  for (int i = 0; i < this->AvailAnimations; i++)
  {
    log::debug(i, ": ", this->Importer->GetAnimationName(i));
  }

  this->AnimationIndex = this->Options.scene.animation.index;
  if (this->AnimationIndex > 0 && this->AnimationIndex >= this->AvailAnimations)
  {
    log::warn(
      "Specified animation index is greater than the highest possible animation index, enabling "
      "the first animation.");
    this->AnimationIndex = 0;
  }
  this->EnableOnlyCurrentAnimation();

  // Recover time ranges for all enabled animations
  this->TimeRange[0] = std::numeric_limits<double>::infinity();
  this->TimeRange[1] = -std::numeric_limits<double>::infinity();
  for (vtkIdType animIndex = 0; animIndex < this->AvailAnimations; animIndex++)
  {
    if (this->Importer->IsAnimationEnabled(animIndex))
    {
      double timeRange[2];
      int nbTimeSteps;
      vtkNew<vtkDoubleArray> timeSteps;

      // Discard timesteps, F3D only cares about elapsed time using time range and deltaTime
      // Specifying a frame rate (60) in the next call is not needed after VTK 9.2.20230603 :
      // VTK_VERSION_CHECK(9, 2, 20230603)
      this->Importer->GetTemporalInformation(animIndex, 60, nbTimeSteps, timeRange, timeSteps);

      // Accumulate time ranges
      this->TimeRange[0] = std::min(timeRange[0], this->TimeRange[0]);
      this->TimeRange[1] = std::max(timeRange[1], this->TimeRange[1]);
      this->HasAnimation = true;
    }
  }
  if (this->TimeRange[0] >= this->TimeRange[1])
  {
    log::warn("Animation(s) time range delta is invalid: [", this->TimeRange[0], ", ",
      this->TimeRange[1], "]. Disabling animation.");
    this->HasAnimation = false;
    return;
  }
  else
  {
    log::debug("Animation(s) time range is: [", this->TimeRange[0], ", ", this->TimeRange[1], "].");
  }
  log::debug("");

  bool autoplay = this->Options.scene.animation.autoplay;
  if (autoplay)
  {
    this->StartAnimation();
  }
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
}

//----------------------------------------------------------------------------
void animationManager::ToggleAnimation()
{
  if (this->HasAnimation && this->Interactor)
  {
    this->Playing = !this->Playing;

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
    }

    if (this->Playing && this->Options.scene.camera.index.has_value())
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
  if (this->Playing)
  {
    std::chrono::steady_clock::time_point currentTick = std::chrono::steady_clock::now();
    if (this->SkipFrame &&
      std::chrono::duration_cast<std::chrono::milliseconds>(currentTick - this->PreviousTick)
          .count() > 1000 * this->DeltaTime)
    {
      log::debug("Unable to process frames fast enough, skipping a frame");
      this->PreviousTick = currentTick;
      return;
    }
    this->PreviousTick = currentTick;

    this->CurrentTime += this->DeltaTime * this->Options.scene.animation.speed_factor;

    // Modulo computation, compute CurrentTime in the time range.
    if (this->CurrentTime < this->TimeRange[0] || this->CurrentTime > this->TimeRange[1])
    {
      auto modulo = [](double val, double mod)
      {
        const double remainder = fmod(val, mod);
        return remainder < 0 ? remainder + mod : remainder;
      };
      this->CurrentTime = this->TimeRange[0] +
        modulo(this->CurrentTime - this->TimeRange[0], this->TimeRange[1] - this->TimeRange[0]);
    }

    if (this->LoadAtTime(this->CurrentTime))
    {
      this->Window.render();
    }

    // Check if camera orbit option is enabled
    if (this->Options.scene.camera.getOrbitTime > 0)
    {
      double orbitTime = this->Options.scene.camera.getOrbitTime;
      double angle = 360.0 * (this->CurrentTime / orbitTime);

      // Calculate new camera position based on orbit
      double radius = 10.0; // Just Example Radius, can be adjusted later
      double x = radius * cos(vtkMath::RadiansFromDegrees(angle));
      double y = radius * sin(vtkMath::RadiansFromDegrees(angle));

      // Update camera position
      vtkCamera* camera = 
        this->Window.GetRenderWindow()->GetRenderers()->GetFirstRenderer()->GetActiveCamera();
      camera->SetPosition(x, y, camera->GetPosition()[2]);
      camera->SetFocalPoint(0, 0, 0); // Example focal point, can be adjusted
    }
  }
}

//----------------------------------------------------------------------------
bool animationManager::LoadAtTime(double timeValue)
{
  assert(this->Importer);
  if (!this->HasAnimation)
  {
    log::warn("No animation available, cannot load a specific animation time");
    return false;
  }

  /* clamp target time to available range */
  if (timeValue < this->TimeRange[0])
  {
    log::warn("Animation time ", timeValue, " is outside of range [", this->TimeRange[0], ", ",
      this->TimeRange[1], "], using ", this->TimeRange[0], ".");
    timeValue = this->TimeRange[0];
  }
  else if (timeValue > this->TimeRange[1])
  {
    log::warn("Animation time ", timeValue, " is outside of range [", this->TimeRange[0], ", ",
      this->TimeRange[1], "], using ", this->TimeRange[1], ".");
    timeValue = this->TimeRange[1];
  }

  this->CurrentTime = timeValue;
  this->CurrentTimeSet = true;
#if VTK_VERSION_NUMBER >= VTK_VERSION_CHECK(9, 3, 20240707)
  if (!this->Importer->UpdateAtTimeValue(this->CurrentTime))
  {
    log::error("Could not load time value: ", this->CurrentTime);
    return false;
  }
#else
  this->Importer->UpdateTimeStep(this->CurrentTime);
#endif

  if (this->Interactor && this->ProgressWidget)
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

// ---------------------------------------------------------------------------------
void animationManager::CycleAnimation()
{
  assert(this->Importer);
  if (this->AvailAnimations <= 0)
  {
    return;
  }
  this->AnimationIndex += 1;

  if (this->AnimationIndex == this->AvailAnimations)
  {
    this->AnimationIndex = -1;
  }

  this->EnableOnlyCurrentAnimation();
  this->LoadAtTime(this->TimeRange[0]);

  vtkRenderWindow* renWin = this->Window.GetRenderWindow();
  vtkF3DRenderer* ren = vtkF3DRenderer::SafeDownCast(renWin->GetRenderers()->GetFirstRenderer());
  ren->SetCheatSheetConfigured(false);
}

// ---------------------------------------------------------------------------------
int animationManager::GetAnimationIndex()
{
  return this->AnimationIndex;
}

// ---------------------------------------------------------------------------------
std::string animationManager::GetAnimationName()
{
  assert(this->Importer);
  if (this->AvailAnimations <= 0)
  {
    return "No animation";
  }

  if (this->AnimationIndex < 0)
  {
    return "All Animations";
  }
  return this->Importer->GetAnimationName(this->AnimationIndex);
}

//----------------------------------------------------------------------------
void animationManager::EnableOnlyCurrentAnimation()
{
  assert(this->Importer);
  for (int i = 0; i < this->AvailAnimations; i++)
  {
    this->Importer->DisableAnimation(i);
  }
  for (int i = 0; i < this->AvailAnimations; i++)
  {
    if (this->AnimationIndex < 0 || i == this->AnimationIndex)
    {
      this->Importer->EnableAnimation(i);
    }
  }
}

//----------------------------------------------------------------------------
std::pair<double, double> animationManager::GetTimeRange()
{
  return std::make_pair(this->TimeRange[0], this->TimeRange[1]);
}
}

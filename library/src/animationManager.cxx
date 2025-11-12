#include "animationManager.h"

#include "interactor_impl.h"
#include "log.h"
#include "macros.h"
#include "options.h"
#include "window_impl.h"

#include "F3DStyle.h"
#include "vtkF3DRenderer.h"

#include <vtkDoubleArray.h>
#include <vtkImporter.h>
#include <vtkProgressBarRepresentation.h>
#include <vtkRenderWindow.h>
#include <vtkRendererCollection.h>
#include <vtkVersion.h>

#include <algorithm>
#include <cmath>
#include <functional>
#include <numeric>

namespace f3d::detail
{
//----------------------------------------------------------------------------
animationManager::animationManager(options& options, window_impl& window)
  : Options(options)
  , Window(window)
{
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
    f3d::color_t color;
    if (!this->Options.ui.animation_progress_color.has_value())
    {
      const auto [r, g, b] = F3DStyle::GetF3DBlue();
      color = color_t(r, g, b);
    }
    else
    {
      color = this->Options.ui.animation_progress_color.value();
    }
    progressRep->SetProgressBarColor(color.r(), color.g(), color.b());
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

  // Reset animation indices before updating
  this->PreparedAnimationIndices.reset();
  this->PrepareForAnimationIndices();

  if (this->AvailAnimations == 0)
  {
    log::debug("No animation available");
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
  this->PrepareForAnimationIndices();
  if (!this->PreparedAnimationIndices.value().empty() && this->Interactor)
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
  }
}

//----------------------------------------------------------------------------
bool animationManager::LoadAtTime(double timeValue)
{
  assert(this->Importer);

  if (this->AvailAnimations == 0)
  {
    log::warn("No animation available, cannot load a specific animation time");
    this->Playing = false;
    return false;
  }

  this->PrepareForAnimationIndices();
  if (this->PreparedAnimationIndices.value().empty())
  {
    return false;
  }

  /* clamp target time to available range */
  // 1 microsecond tolerance so we don't log messages if times are insignificantly close
  constexpr double epsilon = 1e-6;
  if (timeValue < this->TimeRange[0])
  {
    if (this->TimeRange[0] - timeValue > epsilon)
    {
      log::warn("Animation time ", timeValue, " is outside of range [", this->TimeRange[0], ", ",
        this->TimeRange[1], "], using ", this->TimeRange[0], ".");
    }
    timeValue = this->TimeRange[0];
  }
  else if (timeValue > this->TimeRange[1])
  {
    if (timeValue - this->TimeRange[1] > epsilon)
    {
      log::warn("Animation time ", timeValue, " is outside of range [", this->TimeRange[0], ", ",
        this->TimeRange[1], "], using ", this->TimeRange[1], ".");
    }
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
  if (this->AvailAnimations == 0)
  {
    return;
  }

  // F3D_DEPRECATED
  // Remove this in the next major release
  F3D_SILENT_WARNING_PUSH()
  F3D_SILENT_WARNING_DECL(4996, "deprecated-declarations")
  if (this->Options.scene.animation.indices == std::vector<int>{ 0 } &&
    this->Options.scene.animation.index != 0)
  {
    log::warn("scene.animation.index is deprecated, please use "
              "scene.animation.indices instead");
    this->Options.scene.animation.indices = { this->Options.scene.animation.index };
    this->Options.scene.animation.index = 0;
  }
  F3D_SILENT_WARNING_POP()

  // If we started with multi animation or all animations (any negative value means all animations)
  bool negative = std::any_of(this->Options.scene.animation.indices.begin(),
    this->Options.scene.animation.indices.end(), [](int idx) { return idx < 0; });
  if (this->Options.scene.animation.indices.size() > 1 || negative)
  {
    // Then select no animation
    this->Options.scene.animation.indices.clear();
  }
  // If no animation selected
  else if (this->Options.scene.animation.indices.empty())
  {
    // Select the first one
    this->Options.scene.animation.indices = { 0 };
  }
  else
  {
    // If there was only one animation selected, then increment animation index
    this->Options.scene.animation.indices[0]++;

    // If we reach/exceeded the last animation
    if (this->Options.scene.animation.indices[0] >= this->AvailAnimations)
    {
#if VTK_VERSION_NUMBER >= VTK_VERSION_CHECK(9, 4, 20250507)
      // If importer support multi animations and there are multiple animations
      if (this->Importer->GetAnimationSupportLevel() == vtkImporter::AnimationSupportLevel::MULTI &&
        this->AvailAnimations > 1)
#else
      if (this->AvailAnimations > 1)
#endif
      {
        // Then select all
        this->Options.scene.animation.indices.resize(this->AvailAnimations);
        std::iota(this->Options.scene.animation.indices.begin(),
          this->Options.scene.animation.indices.end(), 0);
      }
#if VTK_VERSION_NUMBER >= VTK_VERSION_CHECK(9, 4, 20250507)
      else
      {
        // If not, select none
        this->Options.scene.animation.indices.clear();
      }
#endif
    }
  }

  this->PrepareForAnimationIndices();
  this->LoadAtTime(this->TimeRange[0]);

  vtkRenderWindow* renWin = this->Window.GetRenderWindow();
  vtkF3DRenderer* ren = vtkF3DRenderer::SafeDownCast(renWin->GetRenderers()->GetFirstRenderer());
  ren->SetCheatSheetConfigured(false);
}

// ---------------------------------------------------------------------------------
std::string animationManager::GetAnimationName()
{
  assert(this->Importer);
  if (this->PreparedAnimationIndices.has_value() &&
    this->PreparedAnimationIndices.value().size() > 1)
  {
    std::vector<bool> animCheck(this->AvailAnimations, false);
    for (int idx : this->PreparedAnimationIndices.value())
    {
      if (idx < this->AvailAnimations)
      {
        animCheck[idx] = true;
      }
    }
    return std::none_of(animCheck.begin(), animCheck.end(), std::logical_not<>())
      ? "All animations"
      : "Multi animations";
  }

  if (this->AvailAnimations == 0 || !this->PreparedAnimationIndices.has_value() ||
    this->PreparedAnimationIndices.value().empty() ||
    this->PreparedAnimationIndices.value()[0] >= this->AvailAnimations)
  {
    return "No animation";
  }

  return this->Importer->GetAnimationName(this->PreparedAnimationIndices.value()[0]);
}

//----------------------------------------------------------------------------
void animationManager::PrepareForAnimationIndices()
{
  assert(this->Importer);

  std::vector<int> animIndices = this->Options.scene.animation.indices;

  // F3D_DEPRECATED
  // Remove this in the next major release
  F3D_SILENT_WARNING_PUSH()
  F3D_SILENT_WARNING_DECL(4996, "deprecated-declarations")
  if (animIndices == std::vector<int>{ 0 } && this->Options.scene.animation.index != 0)
  {
    log::warn("scene.animation.index is deprecated, please use "
              "scene.animation.indices instead");
    animIndices = { this->Options.scene.animation.index };
  }
  F3D_SILENT_WARNING_POP()

  // If it contains a negative value, all animations should be selected
  if (std::any_of(animIndices.begin(), animIndices.end(), [](int idx) { return idx < 0; }))
  {
    if (animIndices.size() > 1)
    {
      log::warn("Multiple animation indices have been specified include a negative one, all "
                "animations will be selected");
    }

    animIndices.resize(this->AvailAnimations);
    std::iota(animIndices.begin(), animIndices.end(), 0);
  }

  if (this->PreparedAnimationIndices.has_value() &&
    this->PreparedAnimationIndices.value() == animIndices)
  {
    // Already updated
    return;
  }

  // Do not warn at all if default or empty
  if (!animIndices.empty() && animIndices != std::vector<int>{ 0 })
  {
    if (this->AvailAnimations == 0)
    {
      log::warn(
        "Animation indices have been specified but there are no animation available in this file.");
    }
    else
    {
#if VTK_VERSION_NUMBER >= VTK_VERSION_CHECK(9, 4, 20250507)
      switch (this->Importer->GetAnimationSupportLevel())
      {
        case vtkImporter::AnimationSupportLevel::UNIQUE:
          if (this->Options.scene.animation.indices[0] != 0 ||
            this->Options.scene.animation.indices.size() > 1)
          {
            log::warn("Non-zero or multiple animation indices have been specified but currently "
                      "loaded file does not support it.");
          }
          break;
        case vtkImporter::AnimationSupportLevel::SINGLE:
          if (this->Options.scene.animation.indices.size() > 1)
          {
            log::warn(
              "Multiple animation indices have been specified but currently loaded files may "
              "not support enabling multiple animations");
          }
          break;
        default:
          // NONE is unreachable
          // MULTI there is nothing to warn about
          break;
      }
#endif
    }
  }

  this->PreparedAnimationIndices = animIndices;

  if (this->AvailAnimations == 0)
  {
    return;
  }

  // Disable all animations
  for (int idx = 0; idx < this->AvailAnimations; idx++)
  {
    this->Importer->DisableAnimation(idx);
  }

  // Enable the selected ones
  for (int idx : this->PreparedAnimationIndices.value())
  {
    if (idx >= this->AvailAnimations)
    {
      log::warn("Specified animation index: ", idx, " is not in range [0, ",
        this->AvailAnimations - 1, "], ignoring");
    }
    this->Importer->EnableAnimation(idx);
  }

  // Display currently selected animation
  log::debug("Current animation is: ", this->GetAnimationName());

  // Recover time ranges for all enabled animations
  bool foundAnimation = false;
  this->TimeRange[0] = std::numeric_limits<double>::infinity();
  this->TimeRange[1] = -std::numeric_limits<double>::infinity();
  for (vtkIdType animIndex = 0; animIndex < this->AvailAnimations; animIndex++)
  {
    if (this->Importer->IsAnimationEnabled(animIndex))
    {
      double timeRange[2];
      int nbTimeSteps;
      vtkNew<vtkDoubleArray> timeSteps;
      this->Importer->GetTemporalInformation(animIndex, 0, nbTimeSteps, timeRange, timeSteps);

      // Accumulate time ranges
      this->TimeRange[0] = std::min(timeRange[0], this->TimeRange[0]);
      this->TimeRange[1] = std::max(timeRange[1], this->TimeRange[1]);
      foundAnimation = true;
    }
  }

  if (foundAnimation)
  {
    // Check time range is valid
    if (this->TimeRange[0] > this->TimeRange[1])
    {
      log::warn("Animation(s) time range delta is invalid: [", this->TimeRange[0], ", ",
        this->TimeRange[1], "]. Swapping range.");
      std::swap(this->TimeRange[0], this->TimeRange[1]);
    }
    log::debug(
      "Current animation time range is: [", this->TimeRange[0], ", ", this->TimeRange[1], "].");
  }

  log::debug("");
}

//----------------------------------------------------------------------------
std::pair<double, double> animationManager::GetTimeRange()
{
  // Make sure TimeRange is updated
  this->PrepareForAnimationIndices();

  // Return updated data
  return std::make_pair(this->TimeRange[0], this->TimeRange[1]);
}

//----------------------------------------------------------------------------
unsigned int animationManager::GetNumberOfAvailableAnimations() const
{
  assert(this->AvailAnimations >= 0);
  return static_cast<unsigned int>(this->AvailAnimations);
}
}

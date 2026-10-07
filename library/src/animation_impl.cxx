#include "animation_impl.h"

#include "interactor_impl.h"
#include "log.h"
#include "macros.h"
#include "options.h"
#include "scene_impl.h"
#include "window_impl.h"

#include "F3DStyle.h"
#include "vtkF3DMetaImporter.h"
#include "vtkF3DRenderer.h"

#include <vtkDoubleArray.h>
#include <vtkRenderWindow.h>
#include <vtkRendererCollection.h>
#include <vtkVersion.h>

#include <algorithm>
#include <cmath>
#include <functional>
#include <numeric>

namespace f3d::detail
{

class animation_impl::internals
{
public:
  internals(options& options, window_impl& window)
    : Options(options)
    , Window(window)
  {
  }

  options& Options;
  window_impl& Window;
  vtkF3DMetaImporter* Importer = nullptr;
  interactor_impl* Interactor = nullptr;

  int AvailAnimations = 0;
  int Direction = 1;

  std::optional<std::vector<int>> PreparedAnimationIndices;
  vtkNew<vtkDoubleArray> AnimationTimeSteps;
  double TimeRange[2] = { 0.0, 0.0 };
  bool Playing = false;
  double CurrentTime = 0;
  double DeltaTime = 0;
  bool CurrentTimeSet = false;

  // Dynamic options
  bool Autoplay = false;
  double SpeedFactor = 1.0;
};

//----------------------------------------------------------------------------
animation_impl::animation_impl(options& options, window_impl& window)
  : Internals(std::make_unique<animation_impl::internals>(options, window))
{
}

//----------------------------------------------------------------------------
animation_impl::~animation_impl() = default;

//----------------------------------------------------------------------------
animation& animation_impl::loadTime(double timeValue)
{
  assert(this->Internals->Importer);
  if (this->LoadAtTime(timeValue))
  {
    scene_impl::DisplayAllInfo(this->Internals->Importer, this->Internals->Window);
  }
  return *this;
}

//----------------------------------------------------------------------------
std::pair<double, double> animation_impl::getTimeRange()
{
  // Make sure TimeRange is updated
  this->PrepareForAnimationIndices();

  // Return updated data
  return std::make_pair(this->Internals->TimeRange[0], this->Internals->TimeRange[1]);
}

//----------------------------------------------------------------------------
std::vector<double> animation_impl::getKeyFrames()
{
  this->PrepareForAnimationIndices();

  std::vector<double> keyFrames;
  keyFrames.reserve(this->Internals->AnimationTimeSteps->GetNumberOfTuples());

  for (vtkIdType i = 0; i < this->Internals->AnimationTimeSteps->GetNumberOfTuples(); ++i)
  {
    keyFrames.push_back(this->Internals->AnimationTimeSteps->GetValue(i));
  }

  return keyFrames;
}

//----------------------------------------------------------------------------
unsigned int animation_impl::count() const
{
  assert(this->Internals->AvailAnimations >= 0);
  return static_cast<unsigned int>(this->Internals->AvailAnimations);
}

//----------------------------------------------------------------------------
std::string animation_impl::getName(std::optional<int> index) const
{
  assert(this->Internals->Importer);
  if (index == std::nullopt)
  {
    if (this->Internals->PreparedAnimationIndices.has_value() &&
      this->Internals->PreparedAnimationIndices.value().size() > 1)
    {
      std::vector<bool> animCheck(this->Internals->AvailAnimations, false);
      for (const int idx : this->Internals->PreparedAnimationIndices.value())
      {
        if (idx < this->Internals->AvailAnimations)
        {
          animCheck[idx] = true;
        }
      }
      return std::ranges::none_of(animCheck, std::logical_not<>()) ? "All animations"
                                                                   : "Multi animations";
    }

    if (this->Internals->AvailAnimations == 0 ||
      !this->Internals->PreparedAnimationIndices.has_value() ||
      this->Internals->PreparedAnimationIndices.value().empty() ||
      this->Internals->PreparedAnimationIndices.value()[0] >= this->Internals->AvailAnimations)
    {
      return "No animation";
    }

    return this->Internals->Importer->GetAnimationName(
      this->Internals->PreparedAnimationIndices.value()[0]);
  }

  if (this->Internals->AvailAnimations == 0 || index.value() < 0 ||
    index.value() > this->Internals->AvailAnimations)
  {
    return "No animation";
  }

  return this->Internals->Importer->GetAnimationName(index.value());
}

//----------------------------------------------------------------------------
std::vector<std::string> animation_impl::getNames() const
{
  assert(this->Internals->Importer);

  if (this->Internals->AvailAnimations == 0)
  {
    return {};
  }

  std::vector<std::string> animations(this->Internals->AvailAnimations);

  for (int index = 0; index < this->Internals->AvailAnimations; index++)
  {
    animations[index] = this->Internals->Importer->GetAnimationName(index);
  }

  return animations;
}

//----------------------------------------------------------------------------
animation& animation_impl::toggle(Direction direction)
{
  this->Internals->Direction = (direction == Direction::FORWARD ? 1 : -1);

  this->PrepareForAnimationIndices();
  if (!this->Internals->PreparedAnimationIndices.value().empty() && this->Internals->Interactor)
  {
    this->Internals->Playing = !this->Internals->Playing;

    if (this->Internals->Playing)
    {
      // Initialize time if not already
      if (!this->Internals->CurrentTimeSet)
      {
        this->Internals->CurrentTime = this->Internals->TimeRange[0];
        this->Internals->CurrentTimeSet = true;
      }
    }

    if (this->Internals->Playing && this->Internals->Options.scene.camera.index.has_value())
    {
      this->Internals->Interactor->disableCameraMovement();
    }
    else
    {
      this->Internals->Interactor->enableCameraMovement();
    }
  }

  return *this;
}

//----------------------------------------------------------------------------
animation& animation_impl::start(Direction direction)
{
  if (!this->isPlaying())
  {
    this->toggle(direction);
  }

  return *this;
}

//----------------------------------------------------------------------------
animation& animation_impl::stop()
{
  if (this->isPlaying())
  {
    this->toggle();
  }

  return *this;
}

//----------------------------------------------------------------------------
bool animation_impl::isPlaying()
{
  return this->Internals->Playing;
}

//----------------------------------------------------------------------------
animation::Direction animation_impl::getDirection()
{
  return this->Internals->Direction == 1 ? Direction::FORWARD : Direction::BACKWARD;
}

//----------------------------------------------------------------------------
void animation_impl::SetImporter(vtkF3DMetaImporter* importer)
{
  this->Internals->Importer = importer;
}

//----------------------------------------------------------------------------
void animation_impl::SetInteractor(interactor_impl* interactor)
{
  this->Internals->Interactor = interactor;
}

//----------------------------------------------------------------------------
void animation_impl::SetDeltaTime(double deltaTime)
{
  this->Internals->DeltaTime = deltaTime;
}

//----------------------------------------------------------------------------
void animation_impl::Initialize()
{
  assert(this->Internals->Importer);
  this->Internals->Playing = false;
  this->Internals->CurrentTime = 0;
  this->Internals->CurrentTimeSet = false;

  this->Internals->AvailAnimations = this->Internals->Importer->GetNumberOfAnimations();

  // Reset animation indices before updating
  this->Internals->PreparedAnimationIndices.reset();
  this->Internals->AnimationTimeSteps->Reset();
  this->PrepareForAnimationIndices();

  // Push the animation time range and name to the UI actor
  this->PushAnimationProgress();

  if (this->Internals->AvailAnimations == 0)
  {
    log::debug("No animation available");
    return;
  }
  else
  {
    log::debug("Animation(s) available are:");
  }

  for (int i = 0; i < this->Internals->AvailAnimations; i++)
  {
    log::debug(i, ": ", this->Internals->Importer->GetAnimationName(i));
  }

  if (this->Internals->Autoplay)
  {
    this->start();
  }
}

//----------------------------------------------------------------------------
void animation_impl::Reset()
{
  assert(this->Internals->Importer);
  this->Internals->Playing = false;
  this->Internals->CurrentTime = 0;
  this->Internals->CurrentTimeSet = false;
  this->Internals->AvailAnimations = 0;

  this->Internals->PreparedAnimationIndices.reset();
  this->Internals->AnimationTimeSteps->Reset();

  // No animation is loaded: hide the progress bar
  this->PushAnimationProgress();
}

//----------------------------------------------------------------------------
void animation_impl::Tick()
{
  assert(this->Internals->DeltaTime > 0);
  if (this->Internals->Playing)
  {
    this->Internals->CurrentTime +=
      (this->Internals->DeltaTime * this->Internals->SpeedFactor) * this->Internals->Direction;

    // Modulo computation, compute CurrentTime in the time range.
    if (this->Internals->CurrentTime < this->Internals->TimeRange[0] ||
      this->Internals->CurrentTime > this->Internals->TimeRange[1])
    {
      auto modulo = [](double val, double mod)
      {
        const double remainder = fmod(val, mod);
        return remainder < 0 ? remainder + mod : remainder;
      };
      this->Internals->CurrentTime = this->Internals->TimeRange[0] +
        modulo(this->Internals->CurrentTime - this->Internals->TimeRange[0],
          this->Internals->TimeRange[1] - this->Internals->TimeRange[0]);
    }

    if (this->LoadAtTime(this->Internals->CurrentTime))
    {
      this->Internals->Window.render();
    }
  }
}

//----------------------------------------------------------------------------
void animation_impl::JumpToFrame(int frame, bool relative)
{
  assert(this->Internals->DeltaTime > 0);
  const double frameDuration = (this->Internals->DeltaTime * this->Internals->SpeedFactor);
  const double currentFrame =
    (this->Internals->CurrentTime - this->Internals->TimeRange[0]) / frameDuration;

  double nextFrame = 0;
  if (relative)
  {
    nextFrame = currentFrame + frame;
  }
  else if (frame >= 0)
  {
    nextFrame = frame;
  }
  else
  {
    nextFrame = (this->Internals->TimeRange[1] - this->Internals->TimeRange[0]) / frameDuration;
  }

  this->Internals->CurrentTime = this->Internals->TimeRange[0] +
    (nextFrame * this->Internals->DeltaTime * this->Internals->SpeedFactor);

  if (this->LoadAtTime(this->Internals->CurrentTime))
  {
    this->Internals->Window.render();
  }
}

//----------------------------------------------------------------------------
void animation_impl::JumpToTime(double timeValue, bool relative)
{
  const double target = relative ? this->Internals->CurrentTime + timeValue : timeValue;

  if (this->LoadAtTime(target))
  {
    this->Internals->Window.render();
  }
}

//----------------------------------------------------------------------------
void animation_impl::JumpToKeyFrame(int keyframe, bool relative)
{
  if (this->Internals->AnimationTimeSteps->GetNumberOfTuples() == 0)
  {
    return;
  }

  const int timeStepsAvailable = this->Internals->AnimationTimeSteps->GetNumberOfTuples();

  auto it = std::lower_bound(this->Internals->AnimationTimeSteps->Begin(),
    this->Internals->AnimationTimeSteps->End(), this->Internals->CurrentTime);
  const int closestKeyFrame = (it != this->Internals->AnimationTimeSteps->End())
    ? static_cast<int>(std::distance(this->Internals->AnimationTimeSteps->Begin(), it))
    : timeStepsAvailable - 1;

  int nextKeyFrame = closestKeyFrame;
  if (relative)
  {
    nextKeyFrame += keyframe;
    nextKeyFrame = ((nextKeyFrame % timeStepsAvailable) + timeStepsAvailable) % timeStepsAvailable;
  }
  else
  {
    nextKeyFrame = keyframe > 0 ? std::min(keyframe, timeStepsAvailable - 1) : 0;
    if (0 > keyframe || keyframe > timeStepsAvailable)
    {
      log::warn("Keyframe index ", keyframe, " is outside of range [0-", timeStepsAvailable - 1,
        "], converting to ", nextKeyFrame, " instead.");
    }
  }

  this->Internals->CurrentTime = this->Internals->AnimationTimeSteps->GetValue(nextKeyFrame);

  if (this->LoadAtTime(this->Internals->CurrentTime))
  {
    this->Internals->Window.render();
  }
}

//----------------------------------------------------------------------------
bool animation_impl::LoadAtTime(double timeValue)
{
  assert(this->Internals->Importer);

  if (this->Internals->AvailAnimations == 0)
  {
    log::warn("No animation available, cannot load a specific animation time");
    this->Internals->Playing = false;
    return false;
  }

  this->PrepareForAnimationIndices();
  if (this->Internals->PreparedAnimationIndices.value().empty())
  {
    return false;
  }

  /* clamp target time to available range */
  // 1 microsecond tolerance so we don't log messages if times are insignificantly close
  constexpr double epsilon = 1e-6;
  if (timeValue < this->Internals->TimeRange[0])
  {
    if (this->Internals->TimeRange[0] - timeValue > epsilon)
    {
      log::warn("Animation time ", timeValue, " is outside of range [",
        this->Internals->TimeRange[0], ", ", this->Internals->TimeRange[1], "], using ",
        this->Internals->TimeRange[0], ".");
    }
    timeValue = this->Internals->TimeRange[0];
  }
  else if (timeValue > this->Internals->TimeRange[1])
  {
    if (timeValue - this->Internals->TimeRange[1] > epsilon)
    {
      log::warn("Animation time ", timeValue, " is outside of range [",
        this->Internals->TimeRange[0], ", ", this->Internals->TimeRange[1], "], using ",
        this->Internals->TimeRange[1], ".");
    }
    timeValue = this->Internals->TimeRange[1];
  }
  this->Internals->CurrentTime = timeValue;
  this->Internals->CurrentTimeSet = true;
  if (!this->Internals->Importer->UpdateAtTimeValue(this->Internals->CurrentTime))
  {
    log::error("Could not load time value: ", this->Internals->CurrentTime);
    return false;
  }

  this->Internals->Window.GetRenderer()->UpdateAnimationTime(this->Internals->CurrentTime);

  if (this->Internals->AvailAnimations > 0 && this->Internals->Interactor)
  {
    this->Internals->Interactor->UpdateRendererAfterInteraction();
  }

  return true;
}

// ---------------------------------------------------------------------------------
void animation_impl::CycleAnimation()
{
  assert(this->Internals->Importer);
  if (this->Internals->AvailAnimations == 0)
  {
    return;
  }

  // If we started with multi animation or all animations (any negative value means all animations)
  const bool negative = std::ranges::any_of(
    this->Internals->Options.scene.animation.indices, [](int idx) { return idx < 0; });
  if (this->Internals->Options.scene.animation.indices.size() > 1 || negative)
  {
    // Then select no animation
    this->Internals->Options.scene.animation.indices.clear();
  }
  // If no animation selected
  else if (this->Internals->Options.scene.animation.indices.empty())
  {
    // Select the first one
    this->Internals->Options.scene.animation.indices.emplace_back(0);
  }
  else
  {
    // If there was only one animation selected, then increment animation index
    this->Internals->Options.scene.animation.indices[0]++;

    // If we reach/exceeded the last animation
    if (this->Internals->Options.scene.animation.indices[0] >= this->Internals->AvailAnimations)
    {
#if VTK_VERSION_NUMBER >= VTK_VERSION_CHECK(9, 4, 20250507)
      // If importer support multi animations and there are multiple animations
      if (this->Internals->Importer->GetAnimationSupportLevel() ==
          vtkImporter::AnimationSupportLevel::MULTI &&
        this->Internals->AvailAnimations > 1)
#else
      if (this->Internals->AvailAnimations > 1)
#endif
      {
        // Then select all
        this->Internals->Options.scene.animation.indices.resize(this->Internals->AvailAnimations);
        std::iota(this->Internals->Options.scene.animation.indices.begin(),
          this->Internals->Options.scene.animation.indices.end(), 0);
      }
#if VTK_VERSION_NUMBER >= VTK_VERSION_CHECK(9, 4, 20250507)
      else
      {
        // If not, select none
        this->Internals->Options.scene.animation.indices.clear();
      }
#endif
    }
  }

  this->PrepareForAnimationIndices();
  if (this->LoadAtTime(this->Internals->TimeRange[0]))
  {
    // The loaded animation changed: refresh the progress bar's time range and name
    this->PushAnimationProgress();

    vtkRenderWindow* renWin = this->Internals->Window.GetRenderWindow();
    vtkF3DRenderer* ren = vtkF3DRenderer::SafeDownCast(renWin->GetRenderers()->GetFirstRenderer());
    ren->SetCheatSheetConfigured(false);
  }
}

//----------------------------------------------------------------------------
void animation_impl::PushAnimationProgress()
{
  if (this->Internals->AvailAnimations <= 0)
  {
    // No animation: clear the range so the bar hides itself
    this->Internals->Window.GetRenderer()->SetAnimationProgress({ 0.0, 0.0 }, "", {});
  }
  else
  {
    this->Internals->Window.GetRenderer()->SetAnimationProgress(
      this->getTimeRange(), this->getName(), this->getKeyFrames());
  }
}

//----------------------------------------------------------------------------
void animation_impl::PrepareForAnimationIndices()
{
  assert(this->Internals->Importer);

  std::vector<int> animIndices = this->Internals->Options.scene.animation.indices;

  // If it contains a negative value, all animations should be selected
  if (std::ranges::any_of(animIndices, [](int idx) { return idx < 0; }))
  {
    if (animIndices.size() > 1)
    {
      log::warn("Multiple animation indices have been specified include a negative one, all "
                "animations will be selected");
    }

    animIndices.resize(this->Internals->AvailAnimations);
    std::iota(animIndices.begin(), animIndices.end(), 0);
  }

  if (this->Internals->PreparedAnimationIndices.has_value() &&
    this->Internals->PreparedAnimationIndices.value() == animIndices)
  {
    // Already updated
    return;
  }

  // Do not warn at all if default or empty
  if (!animIndices.empty() && animIndices != std::vector<int>{ 0 })
  {
    if (this->Internals->AvailAnimations == 0)
    {
      log::warn(
        "Animation indices have been specified but there are no animation available in this file.");
    }
    else
    {
#if VTK_VERSION_NUMBER >= VTK_VERSION_CHECK(9, 4, 20250507)
      switch (this->Internals->Importer->GetAnimationSupportLevel())
      {
        case vtkImporter::AnimationSupportLevel::UNIQUE:
          if (this->Internals->Options.scene.animation.indices[0] != 0 ||
            this->Internals->Options.scene.animation.indices.size() > 1)
          {
            log::warn("Non-zero or multiple animation indices have been specified but currently "
                      "loaded file does not support it.");
          }
          break;
        case vtkImporter::AnimationSupportLevel::SINGLE:
          if (this->Internals->Options.scene.animation.indices.size() > 1)
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

  this->Internals->PreparedAnimationIndices = animIndices;

  if (this->Internals->AvailAnimations == 0)
  {
    return;
  }

  // Disable all animations
  for (int idx = 0; idx < this->Internals->AvailAnimations; idx++)
  {
    this->Internals->Importer->DisableAnimation(idx);
  }

  // Enable the selected ones
  for (const int idx : this->Internals->PreparedAnimationIndices.value())
  {
    if (idx >= this->Internals->AvailAnimations)
    {
      log::warn("Specified animation index: ", idx, " is not in range [0, ",
        this->Internals->AvailAnimations - 1, "], ignoring");
    }
    this->Internals->Importer->EnableAnimation(idx);
  }

  // Display currently selected animation
  log::debug("Current animation is: ", this->getName());

  // Recover time ranges for all enabled animations
  bool foundAnimation = false;
  this->Internals->TimeRange[0] = std::numeric_limits<double>::infinity();
  this->Internals->TimeRange[1] = -std::numeric_limits<double>::infinity();
  std::set<double> accumulatedTimeSteps;
  for (vtkIdType animIndex = 0; animIndex < this->Internals->AvailAnimations; animIndex++)
  {
    if (this->Internals->Importer->IsAnimationEnabled(animIndex))
    {
      double timeRange[2];
      int nbTimeSteps;
      this->Internals->Importer->GetTemporalInformation(
        animIndex, timeRange, nbTimeSteps, this->Internals->AnimationTimeSteps);

      // Accumulate timesteps to avoid overwrite
      for (vtkIdType stepIndex = 0;
           stepIndex < this->Internals->AnimationTimeSteps->GetNumberOfTuples(); stepIndex++)
      {
        accumulatedTimeSteps.emplace(this->Internals->AnimationTimeSteps->GetValue(stepIndex));
      }

      // Accumulate time ranges
      this->Internals->TimeRange[0] = std::min(timeRange[0], this->Internals->TimeRange[0]);
      this->Internals->TimeRange[1] = std::max(timeRange[1], this->Internals->TimeRange[1]);
      foundAnimation = true;
    }
  }

  if (foundAnimation)
  {
    // Populate AnimationTimeSteps with accumulated values
    this->Internals->AnimationTimeSteps->Reset();
    const int nbAccumulatedTimeSteps = static_cast<int>(accumulatedTimeSteps.size());
    this->Internals->AnimationTimeSteps->SetNumberOfTuples(nbAccumulatedTimeSteps);
    int index = 0;
    for (const double timeStep : accumulatedTimeSteps)
    {
      this->Internals->AnimationTimeSteps->SetValue(index, timeStep);
      index++;
    }

    assert(this->Internals->TimeRange[0] > this->Internals->TimeRange[1]);
    log::debug("Current animation time range is: [", this->Internals->TimeRange[0], ", ",
      this->Internals->TimeRange[1], "].");
  }

  log::debug("");
}

//----------------------------------------------------------------------------
void animation_impl::SetCheatSheetConfigured(bool configured)
{
  vtkF3DRenderer* ren = this->Internals->Window.GetRenderer();
  ren->SetCheatSheetConfigured(configured);
}

//----------------------------------------------------------------------------
void animation_impl::SetAutoplay(bool enable)
{
  if (this->Internals->Autoplay != enable)
  {
    this->Internals->Autoplay = enable;
    this->SetCheatSheetConfigured(false);
  }
}

//----------------------------------------------------------------------------
void animation_impl::SetSpeedFactor(double speedFactor)
{
  if (this->Internals->SpeedFactor != speedFactor)
  {
    this->Internals->SpeedFactor = speedFactor;
    this->SetCheatSheetConfigured(false);
  }
}

//----------------------------------------------------------------------------
void animation_impl::UpdateDynamicOptions()
{
  this->SetAutoplay(this->Internals->Options.scene.animation.autoplay);
  this->SetSpeedFactor(this->Internals->Options.scene.animation.speed_factor);
}

//----------------------------------------------------------------------------
double animation_impl::GetCurrentTime() const
{
  return this->Internals->CurrentTime;
}
}

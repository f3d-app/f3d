#include "log.h"

#include "init.h"

#include "F3DLog.h"

#include <array>
#include <cmath>
#include <format>
#include <string>
#include <vtkObject.h>
#include <vtkTimerLog.h>

namespace f3d
{
namespace detail
{
log::VerboseLevel convertLevel(F3DLog::Severity level)
{
  switch (level)
  {
    case F3DLog::Severity::Debug:
      return log::VerboseLevel::DEBUG;
    case F3DLog::Severity::Info:
      return log::VerboseLevel::INFO;
    case F3DLog::Severity::Warning:
      return log::VerboseLevel::WARN;
    case F3DLog::Severity::Error:
      return log::VerboseLevel::ERROR;
    case F3DLog::Severity::Quiet:
    default:
      return log::VerboseLevel::QUIET;
  }
}
}

//----------------------------------------------------------------------------
// Progress bar helper function
template<std::size_t N>
static std::string GenerateCLIBarString(
  double progress, int barCount, const std::array<std::string_view, N>& strRamp)
{
  std::string bar;
  double filledBars;
  const double lastBarProgression = std::modf(progress * barCount, &filledBars);
  int totalFilled = static_cast<int>(filledBars);
  for (int i = 0; i < filledBars; i++)
  {
    bar += strRamp.back();
  }
  if (filledBars < barCount)
  {
    const size_t charRampIdx =
      static_cast<size_t>(std::round(lastBarProgression * (strRamp.size() - 1)));
    bar += strRamp[charRampIdx];
    totalFilled++;
  }
  while (totalFilled < barCount)
  {
    bar += strRamp.front();
    totalFilled++;
  }
  return bar;
}

//----------------------------------------------------------------------------
log::ProgressBar::ProgressBar()
  : timer(nullptr)
  , title()
{
  detail::init::initialize();
  F3DLog::StartProgress();
}

//----------------------------------------------------------------------------
log::ProgressBar::~ProgressBar()
{
  F3DLog::EndProgress();
}

//----------------------------------------------------------------------------
void log::ProgressBar::operator()(const float progressPercentage)
{
  constexpr int barCount = 16;
  // hex characters correspond to the filled progression bars.
  // sorted from least filled to mostly filled.
  constexpr std::array charRamp = std::to_array<std::string_view>(
    { " ", "\u258f", "\u258e", "\u258d", "\u258c", "\u258b", "\u258a", "\u2589", "\u2588" });

  const std::string bar = GenerateCLIBarString(progressPercentage, barCount, charRamp);

  const int percentage = std::round(100 * progressPercentage);
  const std::string percentageStr = std::format("{:3}", percentage);
  this->timer->StopTimer();

  const double estimatedTime =
    (progressPercentage <= 1e-4) ? 0 : timer->GetElapsedTime() / progressPercentage;
  const int estimatedMin = estimatedTime / 60;
  const int estimatedSec = static_cast<int>(estimatedTime) % 60;
  const int elapsedMin = timer->GetElapsedTime() / 60;
  const int elapsedSec = static_cast<int>(timer->GetElapsedTime()) % 60;

  const std::string time =
    std::format("{:02}:{:02}/{:02}:{:02}", elapsedMin, elapsedSec, estimatedMin, estimatedSec);

  const std::string progressFormat =
    std::format("{} : {}% |{}| [{}]", this->title, percentageStr, bar, time);
  F3DLog::Progress(progressFormat);
};

//----------------------------------------------------------------------------
void log::printInternal(log::VerboseLevel level, const std::string& str)
{
  detail::init::initialize();

  switch (level)
  {
    case (log::VerboseLevel::DEBUG):
      F3DLog::Print(F3DLog::Severity::Debug, str);
      break;
    case (log::VerboseLevel::INFO):
      F3DLog::Print(F3DLog::Severity::Info, str);
      break;
    case (log::VerboseLevel::WARN):
      F3DLog::Print(F3DLog::Severity::Warning, str);
      break;
    case (log::VerboseLevel::ERROR):
      F3DLog::Print(F3DLog::Severity::Error, str);
      break;
    case (log::VerboseLevel::QUIET):
    default:
      break;
  }
}

//----------------------------------------------------------------------------
void log::debugInternal(const std::string& str)
{
  detail::init::initialize();
  F3DLog::Print(F3DLog::Severity::Debug, str);
}

//----------------------------------------------------------------------------
void log::infoInternal(const std::string& str)
{
  detail::init::initialize();
  F3DLog::Print(F3DLog::Severity::Info, str);
}

//----------------------------------------------------------------------------
void log::warnInternal(const std::string& str)
{
  detail::init::initialize();
  F3DLog::Print(F3DLog::Severity::Warning, str);
}

//----------------------------------------------------------------------------
void log::errorInternal(const std::string& str)
{
  detail::init::initialize();
  F3DLog::Print(F3DLog::Severity::Error, str);
}

//----------------------------------------------------------------------------
void log::setUseColoring(bool use)
{
  detail::init::initialize();
  F3DLog::SetUseColoring(use);
}

//----------------------------------------------------------------------------
void log::setVerboseLevel(log::VerboseLevel level, bool forceStdErr)
{
  detail::init::initialize();

  if (level == log::VerboseLevel::QUIET)
  {
    F3DLog::SetStandardStream(F3DLog::StandardStream::None);
  }
  else
  {
    F3DLog::SetStandardStream(
      forceStdErr ? F3DLog::StandardStream::AlwaysStdErr : F3DLog::StandardStream::Default);
  }

  switch (level)
  {
    case (log::VerboseLevel::DEBUG):
      F3DLog::VerboseLevel = F3DLog::Severity::Debug;
      break;
    case (log::VerboseLevel::INFO):
      F3DLog::VerboseLevel = F3DLog::Severity::Info;
      break;
    case (log::VerboseLevel::WARN):
      F3DLog::VerboseLevel = F3DLog::Severity::Warning;
      break;
    case (log::VerboseLevel::ERROR):
      F3DLog::VerboseLevel = F3DLog::Severity::Error;
      break;
    case (log::VerboseLevel::QUIET):
      F3DLog::VerboseLevel = F3DLog::Severity::Quiet;
      break;
    default:
      break;
  }

  // Display third parties log on Debug level
  vtkObject::SetGlobalWarningDisplay(level == log::VerboseLevel::DEBUG);
}

//----------------------------------------------------------------------------
log::VerboseLevel log::getVerboseLevel()
{
  detail::init::initialize();

  return detail::convertLevel(F3DLog::VerboseLevel);
}

//----------------------------------------------------------------------------
void log::forward(const forward_fn_t& callback)
{
  detail::init::initialize();

  if (callback == nullptr)
  {
    F3DLog::Forward(nullptr);
    return;
  }

  F3DLog::Forward([=](F3DLog::Severity sev, const std::string& msg)
    { callback(detail::convertLevel(sev), msg); });
}
}

#include "log.h"

#include "F3DLog.h"

namespace f3d
{

//----------------------------------------------------------------------------
void log::printInternal(log::VerboseLevel level, const std::string& str)
{
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
  F3DLog::Print(F3DLog::Severity::Debug, str);
}

//----------------------------------------------------------------------------
void log::infoInternal(const std::string& str)
{
  F3DLog::Print(F3DLog::Severity::Info, str);
}

//----------------------------------------------------------------------------
void log::warnInternal(const std::string& str)
{
  F3DLog::Print(F3DLog::Severity::Warning, str);
}

//----------------------------------------------------------------------------
void log::errorInternal(const std::string& str)
{
  F3DLog::Print(F3DLog::Severity::Error, str);
}

//----------------------------------------------------------------------------
void log::setUseColoring(bool use)
{
  F3DLog::SetUseColoring(use);
}

//----------------------------------------------------------------------------
void log::setVerboseLevel(log::VerboseLevel level)
{
  F3DLog::SetQuiet(level == log::VerboseLevel::QUIET);
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
    default:
      break;
  }
}

//----------------------------------------------------------------------------
void log::waitForUser()
{
  F3DLog::WaitForUser();
}
}

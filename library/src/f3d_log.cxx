#include "f3d_log.h"

#include "F3DLog.h"

namespace f3d
{
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
void log::setQuiet(bool quiet)
{
  F3DLog::SetQuiet(quiet);
}

//----------------------------------------------------------------------------
void log::waitForUser()
{
  F3DLog::WaitForUser();
}
}

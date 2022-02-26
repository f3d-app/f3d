#include "f3d_log.h"

#include "vtkF3DLog.h"

namespace f3d
{

//----------------------------------------------------------------------------
void log::infoInternal(const std::string& str)
{
  vtkF3DLog::Print(vtkF3DLog::Severity::Info, str);
}

//----------------------------------------------------------------------------
void log::warnInternal(const std::string& str)
{
  vtkF3DLog::Print(vtkF3DLog::Severity::Warning, str);
}

//----------------------------------------------------------------------------
void log::errorInternal(const std::string& str)
{
  vtkF3DLog::Print(vtkF3DLog::Severity::Error, str);
}

//----------------------------------------------------------------------------
void log::setUseColoring(bool use)
{
  vtkF3DLog::SetUseColoring(use);
}

//----------------------------------------------------------------------------
void log::setQuiet(bool quiet)
{
  vtkF3DLog::SetQuiet(quiet);
}

//----------------------------------------------------------------------------
void log::waitForUser()
{
  vtkF3DLog::WaitForUser();
}
}

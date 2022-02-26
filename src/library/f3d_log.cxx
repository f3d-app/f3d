#include "f3d_log.h"

#include "vtkF3DLog.h"

namespace f3d
{
//----------------------------------------------------------------------------
template<typename... Args>
static void log::info(Args... args)
{
  vtkF3DLog::print(log::Severity::Info, args...);
}

//----------------------------------------------------------------------------
template<typename... Args>
static void log::warn(Args... args)
{
  vtkF3DLog::print(log::Severity::Warning, args...);
}

//----------------------------------------------------------------------------
template<typename... Args>
static void log::error(Args... args)
{
  vtkF3DLog::print(log::Severity::Error, args...);
}

//----------------------------------------------------------------------------
void log::setUseColoring(bool use)
{
  vtkF3DLog::setUseColoring(use);
}

//----------------------------------------------------------------------------
void log::setQuiet(bool quiet)
{
  vtkF3DLog::setQuiet(quiet);
}

//----------------------------------------------------------------------------
void log::waitForUser()
{
  vtkF3DLog::waitForUser();
}
}

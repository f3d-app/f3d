#include <vtkNew.h>
#include <vtkTestUtilities.h>

#include "F3DLog.h"

int TestF3DLog(int argc, char* argv[])
{
  F3DLog::Print(F3DLog::Severity::Info, "Test Info ");
  F3DLog::Print(F3DLog::Severity::Warning, "Test Warning ");
  F3DLog::Print(F3DLog::Severity::Error, "Test Error ");

  F3DLog::SetUseColoring(true);
  F3DLog::Print(F3DLog::Severity::Info, "Test Info ");
  F3DLog::Print(F3DLog::Severity::Warning, "Test Warning ");
  F3DLog::Print(F3DLog::Severity::Error, "Test Error ");

  F3DLog::SetQuiet(true);
  F3DLog::Print(F3DLog::Severity::Info, "Test Info ");
  F3DLog::Print(F3DLog::Severity::Warning, "Test Warning ");
  F3DLog::Print(F3DLog::Severity::Error, "Test Error ");
  return EXIT_SUCCESS;
}

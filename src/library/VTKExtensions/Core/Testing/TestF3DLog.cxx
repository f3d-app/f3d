#include <vtkNew.h>
#include <vtkTestUtilities.h>

#include "F3DLog.h"

int TestF3DLog(int argc, char* argv[])
{
  F3DLog::Print(F3DLog::Severity::Info, "Test Info ");
  F3DLog::Print(F3DLog::Severity::Warning, "Test Warning ");
  F3DLog::Print(F3DLog::Severity::Error, "Test Error ");

  F3DLog::SetUseColoring(true);
  F3DLog::Print(F3DLog::Severity::Info, "Test Info Coloring ");
  F3DLog::Print(F3DLog::Severity::Warning, "Test Warning Coloring ");
  F3DLog::Print(F3DLog::Severity::Error, "Test Error Coloring ");

  F3DLog::SetQuiet(true); // Next print calls should print nothing
  F3DLog::Print(F3DLog::Severity::Info, "Test Info Quiet ");
  F3DLog::Print(F3DLog::Severity::Warning, "Test Warning Quiet ");
  F3DLog::Print(F3DLog::Severity::Error, "Test Error Quiet ");

  F3DLog::WaitForUser(); // Without the object factory created, this is expected to have no effect
  return EXIT_SUCCESS;
}

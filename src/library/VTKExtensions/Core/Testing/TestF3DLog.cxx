#include <vtkNew.h>
#include <vtkTestUtilities.h>

#include "F3DLog.h"

int TestF3DLog(int argc, char* argv[])
{
  F3DLog::Print(F3DLog::Severity::Warning, "Test Output");
  return EXIT_SUCCESS;
}

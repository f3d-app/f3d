#include "F3DLog.h"

int TestF3DLog(int argc, char* argv[])
{
  F3DLog::SetUseColoring(false);
  F3DLog::Print(F3DLog::Severity::Debug, "Test Debug ");
  F3DLog::Print(F3DLog::Severity::Info, "Test Info ");
  F3DLog::Print(F3DLog::Severity::Warning, "Test Warning ");
  F3DLog::Print(F3DLog::Severity::Error, "Test Error\n");

  F3DLog::VerboseLevel = F3DLog::Severity::Debug;
  F3DLog::Print(F3DLog::Severity::Debug, "Test Debug ");
  F3DLog::Print(F3DLog::Severity::Info, "Test Info ");
  F3DLog::Print(F3DLog::Severity::Warning, "Test Warning ");
  F3DLog::Print(F3DLog::Severity::Error, "Test Error\n");

  F3DLog::VerboseLevel = F3DLog::Severity::Warning;
  F3DLog::Print(F3DLog::Severity::Debug, "Test Debug ");
  F3DLog::Print(F3DLog::Severity::Info, "Test Info ");
  F3DLog::Print(F3DLog::Severity::Warning, "Test Warning ");
  F3DLog::Print(F3DLog::Severity::Error, "Test Error\n");

  F3DLog::VerboseLevel = F3DLog::Severity::Error;
  F3DLog::Print(F3DLog::Severity::Debug, "Test Debug ");
  F3DLog::Print(F3DLog::Severity::Info, "Test Info ");
  F3DLog::Print(F3DLog::Severity::Warning, "Test Warning ");
  F3DLog::Print(F3DLog::Severity::Error, "Test Error\n");

  F3DLog::VerboseLevel = F3DLog::Severity::Info;
  F3DLog::SetStandardStream(F3DLog::StandardStream::None); // Next print calls should print nothing
  F3DLog::Print(F3DLog::Severity::Debug, "Test Debug Quiet ");
  F3DLog::Print(F3DLog::Severity::Info, "Test Info Quiet ");
  F3DLog::Print(F3DLog::Severity::Warning, "Test Warning Quiet ");
  F3DLog::Print(F3DLog::Severity::Error, "Test Error Quiet\n");
  F3DLog::SetStandardStream(F3DLog::StandardStream::Default);

  // Without the object factory created, this is expected to have no effect
  F3DLog::SetUseColoring(true);

  F3DLog::Print(F3DLog::Severity::Debug, "Test Debug Coloring ");
  F3DLog::Print(F3DLog::Severity::Info, "Test Info Coloring ");
  F3DLog::Print(F3DLog::Severity::Warning, "Test Warning Coloring ");
  F3DLog::Print(F3DLog::Severity::Error, "Test Error Coloring\n");

  return EXIT_SUCCESS;
}

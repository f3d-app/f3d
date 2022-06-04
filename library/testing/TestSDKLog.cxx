#include <log.h>

int TestSDKLog(int argc, char* argv[])
{
  f3d::log::setUseColoring(false);
  f3d::log::debug("Test Debug");
  f3d::log::info("Test Info");
  f3d::log::warn("Test Warning");
  f3d::log::error("Test Error");
  
  f3d::log::setVerboseLevel(f3d::log::VerboseLevel::WARN); // Next log calls should print warning and error only
  f3d::log::debug("Test Debug");
  f3d::log::info("Test Info");
  f3d::log::warn("Test Warning");
  f3d::log::error("Test Error");

  f3d::log::setVerboseLevel(f3d::log::VerboseLevel::ERROR); // Next log calls should print error only
  f3d::log::debug("Test Debug");
  f3d::log::info("Test Info");
  f3d::log::warn("Test Warning");
  f3d::log::error("Test Error");

  f3d::log::setVerboseLevel(f3d::log::VerboseLevel::QUIET); // Next log calls should print nothing
  f3d::log::debug("Test Debug Quiet");
  f3d::log::info("Test Info Quiet");
  f3d::log::warn("Test Warning Quiet");
  f3d::log::error("Test Error Quiet");

  f3d::log::setVerboseLevel(f3d::log::VerboseLevel::DEBUG); // Next log calls should all be printed
  f3d::log::setUseColoring(true);
  f3d::log::info("Test Debug Coloring");
  f3d::log::info("Test Info Coloring");
  f3d::log::warn("Test Warning Coloring");
  f3d::log::error("Test Error Coloring");

  f3d::log::waitForUser(); // This just returns immediately in testing environment
  return EXIT_SUCCESS;
}

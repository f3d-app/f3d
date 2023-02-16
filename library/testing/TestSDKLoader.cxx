#include <engine.h>
#include <interactor.h>
#include <loader.h>
#include <log.h>
#include <window.h>

#include <iostream>

int TestSDKLoader(int argc, char* argv[])
{
  f3d::log::setVerboseLevel(f3d::log::VerboseLevel::DEBUG);
  f3d::engine eng(f3d::window::Type::NONE);
  f3d::loader& load = eng.getLoader();

  // TODO Rework
  return EXIT_SUCCESS;
}

#include <f3d/engine.h>
#include <f3d/log.h>

int main()
{
  f3d::engine::autoloadPlugins();

  f3d::engine eng = f3d::engine::createNone();

  f3d::log::info("F3D engine is loaded");

  return 0;
}

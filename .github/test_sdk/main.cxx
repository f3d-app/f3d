#include <f3d/engine.h>
#include <f3d/image.h>
#include <f3d/interactor.h>
#include <f3d/loader.h>
#include <f3d/log.h>
#include <f3d/options.h>
#include <f3d/window.h>

int main()
{
  f3d::engine eng(f3d::window::Type::NONE);

  f3d::log::info("F3D engine is loaded");

  return 0;
}

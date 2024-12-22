#include <f3d/engine.h>
#include <f3d/image.h>
#include <f3d/log.h>
#include <f3d/window.h>

int main(int argc, char** argv)
{
  // Check inputs
  if (argc != 2)
  {
    return EXIT_FAILURE;
  }

  // Load static plugins
  f3d::engine::autoloadPlugins();

  // Enable debug logging
  f3d::log::setVerboseLevel(f3d::log::VerboseLevel::DEBUG);

  // Create a native window engine
  f3d::engine eng = f3d::engine::create();

  // add a model
  eng.getScene().add(std::string(argv[1]));

  // Render
  f3d::window& win = eng.getWindow();
  win.render();

  // Start interaction and stop it after one second
  f3d::interactor& inter = eng.getInteractor();
  inter.start(1, [&inter]() { inter.stop(); });

  // Actual call would look like this
  // inter.start();

  return EXIT_SUCCESS;
}

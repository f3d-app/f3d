#include <f3d/engine.h>
#include <f3d/image.h>
#include <f3d/log.h>
#include <f3d/options.h>
#include <f3d/window.h>

#include <filesystem>

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
  f3d::engine eng(f3d::window::Type::NATIVE);

  // Load all files from provided directory as geometries
  f3d::loader& load = eng.getLoader();
  for (auto& entry : std::filesystem::directory_iterator(argv[1]))
  {
    load.loadGeometry(entry.path().string());
  }

  // Render
  f3d::window& win = eng.getWindow();
  win.render();

  // Create a timer to stop interaction after one second
  f3d::interactor& inter = eng.getInteractor();
  inter.createTimerCallBack(1000, [&inter]() { inter.stop(); });

  // Start interaction
  inter.start();

  return EXIT_SUCCESS;
}

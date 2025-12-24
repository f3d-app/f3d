#include <f3d/engine.h>
#include <f3d/image.h>
#include <f3d/log.h>
#include <f3d/window.h>

#include <filesystem>

int main(int argc, char** argv)
{
  // Check inputs
  if (argc < 2)
  {
    return EXIT_FAILURE;
  }

  // Load static plugins
  f3d::engine::autoloadPlugins();

  // Enable debug logging
  f3d::log::setVerboseLevel(f3d::log::VerboseLevel::DEBUG);

  // Create a native window engine
  f3d::engine eng = f3d::engine::create();

  // Add all files from provided directory
  f3d::scene& sce = eng.getScene();
  for (auto& entry : std::filesystem::directory_iterator(argv[1]))
  {
    sce.add(entry.path().string());
  }

  // Render
  f3d::window& win = eng.getWindow();
  win.render();

  // Start interaction and stop it after one second
  f3d::interactor& inter = eng.getInteractor();

  if (argc > 2)
  {
    // For testing purposes only, shutdown the example after `timeout` seconds
    try
    {
      int timeout = std::stoi(argv[2]);
      inter.start(timeout, [&inter]() { inter.stop(); });
    }
    catch (const std::exception& e)
    {
      std::cout << e.what() << std::endl;
      return EXIT_FAILURE;
    }
  }
  else
  {
    inter.start();
  }

  return EXIT_SUCCESS;
}

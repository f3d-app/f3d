#include <f3d/engine.h>
#include <f3d/image.h>
#include <f3d/window.h>

#include <iostream>

int main(int argc, char** argv)
{
  // Check inputs
  if (argc != 3)
  {
    return EXIT_FAILURE;
  }

  // Load static plugins
  f3d::engine::autoloadPlugins();

  // Create a offscreen window engine
  f3d::engine eng(f3d::window::Type::NATIVE_OFFSCREEN);

  // Load a model
  f3d::loader& load = eng.getLoader();
  load.addFile(std::string(argv[1])).loadFile();

  // Set the window size and render to an image
  f3d::image img = eng.getWindow().setSize(300, 300).renderToImage();

  // Save the image to a file
  img.save(std::string(argv[2]));

  return EXIT_SUCCESS;
}

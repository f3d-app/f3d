#include <f3d/engine.h>
#include <f3d/image.h>
#include <f3d/log.h>
#include <f3d/options.h>
#include <f3d/scene.h>
#include <f3d/window.h>

#include <cstdlib>
#include <exception>
#include <iostream>
#include <string>

namespace
{
// Parse optional thumbnail size:
//   argv[3]      -> square size (N x N)
//   argv[3,4]    -> width height
bool ParseSize(int argc, char** argv, int& width, int& height)
{
  // Default thumbnail size
  width = 512;
  height = 512;

  if (argc == 3)
  {
    // Just use the default
    return true;
  }

  try
  {
    if (argc == 4)
    {
      // Single integer: square thumbnail
      const int size = std::stoi(argv[3]);
      if (size <= 0)
      {
        return false;
      }
      width = size;
      height = size;
      return true;
    }
    if (argc == 5)
    {
      width = std::stoi(argv[3]);
      height = std::stoi(argv[4]);
      if (width <= 0 || height <= 0)
      {
        return false;
      }
      return true;
    }
  }
  catch (const std::exception&)
  {
    return false;
  }

  return false;
}

void PrintUsage(const char* exe)
{
  std::cerr << "Usage:\n"
            << "  " << exe << " <input-model> <output-image> [size]\n"
            << "  " << exe << " <input-model> <output-image> <width> <height>\n\n"
            << "Examples:\n"
            << "  " << exe << " model.glb thumb.png        # 512x512 (default)\n"
            << "  " << exe << " model.glb thumb.png 256    # 256x256\n"
            << "  " << exe << " model.glb thumb.png 512 256\n";
}
}

int main(int argc, char** argv)
{
  try
  {
    if (argc < 3 || argc > 5)
    {
      ::PrintUsage(argv[0]);
      return EXIT_FAILURE;
    }

    const std::string inputFile = argv[1];
    const std::string outputFile = argv[2];

    int width = 0;
    int height = 0;
    if (!::ParseSize(argc, argv, width, height))
    {
      f3d::log::error("Invalid thumbnail size.");
      ::PrintUsage(argv[0]);
      return EXIT_FAILURE;
    }

    // Load static/native plugins
    f3d::engine::autoloadPlugins();

    // Create an offscreen engine
    f3d::engine eng = f3d::engine::create(true);

    f3d::options& opt = eng.getOptions();

    // No UI overlays in thumbnails
    opt.ui.axis = false;
    opt.ui.fps = false;
    opt.ui.filename = false;
    opt.ui.metadata = false;
    opt.ui.console = false;
    opt.ui.cheatsheet = false;

    // Neutral background, no grid or skybox
    opt.render.grid.enable = false;
    opt.render.background.skybox = false;
    opt.render.background.color = { 0.15, 0.15, 0.15 }; // dark neutral gray

    // Slightly stronger lighting so assets read well at small sizes
    opt.render.light.intensity = 1.2;

    // Post-processing: AA + AO for better thumbnails
    opt.render.effect.antialiasing.enable = true;
    opt.render.effect.antialiasing.mode = "ssaa";
    opt.render.effect.ambient_occlusion = true;

    // Add a model to the scene
    eng.getScene().add(inputFile);

    // Render offscreen to an image
    f3d::window& win = eng.getWindow();
    win.setSize(width, height);

    // You can make the background transparent by setting noBackground to true in the renderToImage
    // call.
    f3d::image img = win.renderToImage(false);

    // Save the resulting image
    img.save(outputFile);

    return EXIT_SUCCESS;
  }
  catch (const std::exception& ex)
  {
    f3d::log::error("F3D thumbnail example encountered an unexpected exception:");
    f3d::log::error(ex.what());
    return EXIT_FAILURE;
  }
}

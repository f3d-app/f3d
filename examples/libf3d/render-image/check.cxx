#include <f3d/image.h>

int main(int argc, char** argv)
{
  // Check inputs
  if (argc != 3)
  {
    return EXIT_FAILURE;
  }

  // Open images
  f3d::image img0(argv[1]);
  f3d::image img1(argv[2]);

  // Compare them
  return img0.psnr(img1) >= 30.0 ? EXIT_SUCCESS : EXIT_FAILURE;
}

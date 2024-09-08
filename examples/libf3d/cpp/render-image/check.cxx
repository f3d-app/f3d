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
  double error;
  return img0.compare(img1, 50, error) ? EXIT_SUCCESS : EXIT_FAILURE;
}

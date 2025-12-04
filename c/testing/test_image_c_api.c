#include <image_c_api.h>

#include <stdio.h>
#include <string.h>

int test_image_c_api()
{
  int failed = 0;

  f3d_image_t* img = f3d_image_new_params(800, 600, 3, BYTE);
  if (!img)
  {
    puts("[ERROR] Failed to create image");
    return 1;
  }

  unsigned int width = f3d_image_get_width(img);
  unsigned int height = f3d_image_get_height(img);

  if (width != 800 || height != 600)
  {
    puts("[ERROR] Image size should be 800x600");
    failed++;
  }

  unsigned int channels = f3d_image_get_channel_count(img);
  if (channels != 3)
  {
    puts("[ERROR] Image should have 3 channels");
    failed++;
  }

  f3d_image_set_metadata(img, "Author", "TestUser");

  const char* author = f3d_image_get_metadata(img, "Author");
  if (!author || strcmp(author, "TestUser") != 0)
  {
    puts("[ERROR] Metadata 'Author' should be 'TestUser'");
    failed++;
  }

  unsigned int count;
  char** metadata_keys = f3d_image_all_metadata(img, &count);
  if (metadata_keys)
  {
    f3d_image_free_metadata_keys(metadata_keys, count);
  }

  f3d_image_t* ref_img = f3d_image_new_params(800, 600, 3, BYTE);
  if (ref_img)
  {
    double error = f3d_image_compare(img, ref_img);
    if (error > 0.05)
    {
      puts("[ERROR] Images should be similar");
      failed++;
    }

    f3d_image_delete(ref_img);
  }

  f3d_image_delete(img);

  return failed;
}

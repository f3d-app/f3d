#include <image_c_api.h>

#include <stdio.h>

int test_image_stream()
{
  f3d_image_t* img = f3d_image_new_params(800, 600, 3, BYTE);
  if (!img)
  {
    puts("[ERROR] Failed to create image");
    return 1;
  }

  unsigned int count;
  unsigned char* tempBuffer = f3d_image_save_buffer(NULL, PNG, &count); // this shouldn't crash
  if (tempBuffer != NULL)
  {
    f3d_image_delete(img);
    return 1;
  }

  f3d_image_t* temp_image_stream = f3d_image_new_stream(tempBuffer, 0);
  if (temp_image_stream != NULL)
  {
    f3d_image_delete(img);
    return 1;
  }

  unsigned int buffer_size;
  unsigned char* buffer = f3d_image_save_buffer(img, PNG, &buffer_size);
  if (buffer)
  {
    f3d_image_t* image_stream = f3d_image_new_stream(buffer, buffer_size);

    if (image_stream)
    {
      double compare_error = f3d_image_compare(img, image_stream);
      f3d_image_free_buffer(buffer);
      f3d_image_delete(image_stream);

      if (compare_error != 0)
      {
        f3d_image_delete(img);
        return 1;
      }
    }
    else
    {
      f3d_image_delete(img);
      f3d_image_delete(image_stream);
      return 1;
    }
  }

  f3d_image_delete(img);
  return 0;
}

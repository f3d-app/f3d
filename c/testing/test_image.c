#include <image_c_api.h>

#include <stdio.h>

int test_image()
{
  f3d_image_t* img_empty = f3d_image_new_empty();
  if (img_empty)
  {
    f3d_image_delete(img_empty);
  }

  f3d_image_t* img = f3d_image_new_params(800, 600, 3, BYTE);
  if (!img)
  {
    puts("[ERROR] Failed to create image");
    return 1;
  }

  unsigned int width = f3d_image_get_width(img);
  (void)width;
  unsigned int height = f3d_image_get_height(img);
  (void)height;
  unsigned int channels = f3d_image_get_channel_count(img);
  (void)channels;
  unsigned int type = f3d_image_get_channel_type(img);
  (void)type;
  unsigned int type_size = f3d_image_get_channel_type_size(img);
  (void)type_size;

  void* content = f3d_image_get_content(img);
  if (content)
  {
    f3d_image_set_content(img, content);
  }

  double pixel[3];
  f3d_image_get_normalized_pixel(img, 0, 0, pixel);

  f3d_image_set_metadata(img, "Author", "TestUser");
  const char* author = f3d_image_get_metadata(img, "Author");
  (void)author;

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
    (void)error;

    int is_equal = f3d_image_equals(img, ref_img);
    (void)is_equal;

    int is_not_equal = f3d_image_not_equals(img, ref_img);
    (void)is_not_equal;

    f3d_image_delete(ref_img);
  }

  unsigned int buffer_size;
  unsigned char* buffer = f3d_image_save_buffer(img, PNG, &buffer_size);
  if (buffer)
  {
    f3d_image_free_buffer(buffer);
  }

  const char* tmp_path = "/tmp/f3d_test_image.png";
  f3d_image_save(img, tmp_path, PNG);

  f3d_image_t* img_from_file = f3d_image_new_path(tmp_path);
  if (img_from_file)
  {
    f3d_image_delete(img_from_file);
  }

  const char* text = f3d_image_to_terminal_text_string(img);
  (void)text;

  f3d_image_to_terminal_text(img, stdout);

  unsigned int format_count = f3d_image_get_supported_formats_count();
  (void)format_count;
  const char** formats = f3d_image_get_supported_formats();
  (void)formats;

  f3d_image_t* img2 = f3d_image_new_params(100, 100, 4, BYTE);
  if (img2)
  {
    f3d_image_delete(img2);
  }

  f3d_image_delete(img);
  return 0;
}

#include "pseudo_unit_test.h"

#include <image_c_api.h>
#include <string.h>

int test_image()
{
  f3d_test_t test;
  f3d_test_init(&test);

  f3d_image_t* img_empty = f3d_image_new_empty();
  f3d_test_check(&test, "empty image created", img_empty != NULL);
  if (img_empty)
  {
    f3d_image_delete(img_empty);
  }

  f3d_image_t* img = f3d_image_new_params(800, 600, 3, BYTE);
  f3d_test_check(&test, "image created with params", img != NULL);
  if (!img)
  {
    return f3d_test_result(&test);
  }

  // this shouldn't crash, no valid image is expected back
  f3d_image_t* img_wrong_path = f3d_image_new_path("/non/existent/path/image.png");
  f3d_test_check(&test, "loading nonexistent path returns null", img_wrong_path == NULL);

  unsigned int width = f3d_image_get_width(img);
  f3d_test_check_int(&test, "width matches params", width, 800);

  unsigned int height = f3d_image_get_height(img);
  f3d_test_check_int(&test, "height matches params", height, 600);

  unsigned int channels = f3d_image_get_channel_count(img);
  f3d_test_check_int(&test, "channel count matches params", channels, 3);

  int type = f3d_image_get_channel_type(img);
  f3d_test_check_int(&test, "channel type matches params", type, BYTE);

  unsigned int type_size = f3d_image_get_channel_type_size(img);
  f3d_test_check_int(&test, "channel type size is 1 byte", type_size, 1);

  void* content = f3d_image_get_content(img);
  f3d_test_check(&test, "get_content returns non-null buffer", content != NULL);
  if (content)
  {
    // just a roundtrip, no observable state change beyond not crashing
    f3d_image_set_content(img, content);
  }

  double pixel[3];
  int pixel_ret = f3d_image_get_normalized_pixel(img, 0, 0, pixel);
  f3d_test_check_int(&test, "get_normalized_pixel at valid coordinates succeeds", pixel_ret, 1);

  f3d_image_set_metadata(img, "Author", "TestUser");
  const char* author = f3d_image_get_metadata(img, "Author");
  f3d_test_check(&test, "metadata roundtrip returns set value",
    author != NULL && strcmp(author, "TestUser") == 0);

  unsigned int count;
  char** metadata_keys = f3d_image_all_metadata(img, &count);
  f3d_test_check(&test, "all_metadata includes the key we set", count >= 1 && metadata_keys != NULL);
  if (metadata_keys)
  {
    f3d_image_free_metadata_keys(metadata_keys, count);
  }

  /* comparing img against itself avoids any assumption about whether a
   * freshly-allocated image's pixel buffer is zero-initialized, while still
   * exercising the compare/equals/not_equals bindings meaningfully */
  double error = f3d_image_compare(img, img);
  f3d_test_check_double(
    &test, "compare() of an image with itself gives zero error", error, 0.0, 1e-9);

  int is_equal = f3d_image_equals(img, img);
  f3d_test_check(&test, "equals() true for an image compared with itself", is_equal != 0);

  int is_not_equal = f3d_image_not_equals(img, img);
  f3d_test_check(&test, "not_equals() false for an image compared with itself", is_not_equal == 0);

  // this shouldn't crash, and should not return a buffer for a null image
  unsigned char* tempBuffer = f3d_image_save_buffer(NULL, PNG, &count);
  f3d_test_check(&test, "save_buffer on null image returns null", tempBuffer == NULL);

  unsigned int buffer_size;
  unsigned char* buffer = f3d_image_save_buffer(img, PNG, &buffer_size);
  f3d_test_check(
    &test, "save_buffer on valid image returns a buffer", buffer != NULL && buffer_size > 0);
  if (buffer)
  {
    f3d_image_free_buffer(buffer);
  }

  const char* tmp_path = "/tmp/f3d_test_image.png";
  int save_ret = f3d_image_save(img, tmp_path, PNG);
  f3d_test_check_int(&test, "save with valid path and image succeeds", save_ret, 1);

  const char* invalid_path = NULL;
  int ret = f3d_image_save(img, invalid_path, PNG);
  f3d_test_check_int(&test, "save with null path reports failure)", ret, 0);

  f3d_image_t* img_from_file = f3d_image_new_path(tmp_path);
  f3d_test_check(&test, "loading the just-saved image succeeds", img_from_file != NULL);
  if (img_from_file)
  {
    f3d_image_delete(img_from_file);
  }

  const char* text = f3d_image_to_terminal_text_string(img);
  f3d_test_check(&test, "terminal text string is non-null", text != NULL);

  unsigned int format_count = f3d_image_get_supported_formats_count();
  f3d_test_check(&test, "at least one supported format is reported", format_count > 0);

  const char** formats = f3d_image_get_supported_formats();
  f3d_test_check(&test, "supported formats list is non-null", formats != NULL);

  f3d_image_t* img2 = f3d_image_new_params(100, 100, 4, BYTE);
  f3d_test_check(&test, "second image created with different params", img2 != NULL);
  if (img2)
  {
    f3d_test_check_int(
      &test, "second image channel count matches params", f3d_image_get_channel_count(img2), 4);
    f3d_image_delete(img2);
  }

  f3d_image_delete(img);
  return f3d_test_result(&test);
}

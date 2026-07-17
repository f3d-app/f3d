#include "pseudo_unit_test.h"

#include <image_c_api.h>
#include <string.h>

int test_image()
{
  f3d_test_t test;
  f3d_test_init(&test);

  // f3d_image_new_empty tests
  f3d_image_t* image = f3d_image_new_empty();
  f3d_test_check_ptr(&test, "image_new_empty()", image);

  // f3d_image_new_params tests
  f3d_image_t* params = f3d_image_new_params(640, 480, 3, BYTE);
  f3d_test_check_ptr(&test, "image_new_params()", params);

  // f3d_image_new_path tests
  // f3d_test_check_null(&test, "image_new_path(NULL)", f3d_image_new_path(NULL));

  f3d_image_t* world = f3d_image_new_path(F3D_TESTING_DATA_DIR "world.png");
  f3d_test_check_ptr(&test, "image_new_path(valid)", world);

  f3d_test_check_null(&test, "image_new_path(invalid path)", f3d_image_new_path("invalid.png"));

  f3d_test_check_null(&test, "image_new_path(invalid image)", f3d_image_new_path(F3D_TESTING_DATA_DIR "invalid.png"));

  //todoo as may have to make changes in the test to check for valid image formats and invalid image formats
  //no null checks 




  


  return f3d_test_result(&test);
}

#include "pseudo_unit_test.h"
#include <context_c_api.h>

int test_context()
{
  f3d_test_t test;
  f3d_test_init(&test);

  // f3d_context_glx tests
  f3d_context_t* context = f3d_context_glx();

  #ifdef F3D_TESTING_ENABLE_GLX_TESTS
  f3d_test_check_ptr(&test, "context_glx()", context);
  #else
  f3d_test_check_null(&test, "context_glx() unavailable", context);
  #endif

  if (context)
  {
    f3d_context_delete(context);
  }

  // f3d_context_wgl tests
  context = f3d_context_wgl();

  #ifdef F3D_TESTING_ENABLE_WGL_TESTS
  f3d_test_check_ptr(&test, "context_wgl()", context);
  #else
  f3d_test_check_null(&test, "context_wgl() unavailable", context);
  #endif

  if (context)
  {
    f3d_context_delete(context);
  }

  // f3d_context_cocoa tests
  context = f3d_context_cocoa();

  #ifdef __APPLE__
  f3d_test_check_ptr(&test, "context_cocoa()", context);
  #else
  f3d_test_check_null(&test, "context_cocoa() unavailable", context);
  #endif

  if (context)
  {
    f3d_context_delete(context);
  }

  // f3d_context_egl tests
  context = f3d_context_egl();

  #ifdef F3D_TESTING_ENABLE_EGL_TESTS
  f3d_test_check_ptr(&test, "context_egl()", context);
  #else
  f3d_test_check_null(&test, "context_egl() unavailable", context);
  #endif

  if (context)
  {
    f3d_context_delete(context);
  }

  // f3d_context_osmesa tests
  context = f3d_context_osmesa();

  #ifdef F3D_TESTING_ENABLE_OSMESA_TESTS
  f3d_test_check_ptr(&test, "context_osmesa()", context);
  #else
  f3d_test_check_null(&test, "context_osmesa() unavailable", context);
  #endif

  if (context)
  {
    f3d_context_delete(context);
  }

  // f3d_context_get_symbol tests
  f3d_test_check_null(&test, "context_get_symbol(NULL library)", f3d_context_get_symbol(NULL, "invalid"));
  f3d_test_check_null(&test, "context_get_symbol(NULL symbol)", f3d_context_get_symbol("invalid", NULL));

  context = f3d_context_get_symbol("invalid", "invalid");
  f3d_test_check_null(&test, "context_get_symbol(invalid library)", context);

  context = f3d_context_get_symbol("libGLX.so", "invalid");
  f3d_test_check_null(&test, "context_get_symbol(invalid symbol)", context);

  #ifdef F3D_TESTING_ENABLE_GLX_TESTS
  context = f3d_context_get_symbol("libGLX.so", "glXGetProcAddress");
  f3d_test_check_ptr(&test, "context_get_symbol(valid)", context);

  if (context)
  {
    f3d_context_delete(context);
  }
  #endif

  return f3d_test_result(&test);
}

#include "pseudo_unit_test.h"

#include <context_c_api.h>

int test_context()
{
  f3d_test_t test;
  f3d_test_init(&test);

  f3d_context_t* context;

#ifdef F3D_TESTING_ENABLE_GLX_TESTS
  context = f3d_context_glx();
  f3d_test_check(&test, "GLX context creation succeeds", context != NULL);
  if (context)
  {
    f3d_context_delete(context);
  }
#else
  context = f3d_context_glx();
  if (context)
  {
    f3d_context_delete(context);
  }
#endif

#ifdef F3D_TESTING_ENABLE_EGL_TESTS
  context = f3d_context_egl();
  f3d_test_check(&test, "EGL context creation succeeds", context != NULL);
  if (context)
  {
    f3d_context_delete(context);
  }
#else
  context = f3d_context_egl();
  if (context)
  {
    f3d_context_delete(context);
  }
#endif

#ifdef F3D_TESTING_ENABLE_WGL_TESTS
  context = f3d_context_wgl();
  f3d_test_check(&test, "WGL context creation succeeds", context != NULL);
  if (context)
  {
    f3d_context_delete(context);
  }
#else
  context = f3d_context_wgl();
  if (context)
  {
    f3d_context_delete(context);
  }
#endif

  context = f3d_context_cocoa();
  if (context)
  {
    f3d_context_delete(context);
  }

#ifdef F3D_TESTING_ENABLE_OSMESA_TESTS
  context = f3d_context_osmesa();
  f3d_test_check(&test, "OSMesa context creation succeeds", context != NULL);
  if (context)
  {
    f3d_context_delete(context);
  }
#else
  context = f3d_context_osmesa();
  if (context)
  {
    f3d_context_delete(context);
  }
#endif

  return f3d_test_result(&test);
}
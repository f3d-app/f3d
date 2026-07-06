#include "pseudo_unit_test.h"

#include <context_c_api.h>

int test_context()
{
  f3d_test_t test;
  f3d_test_init(&test);

  f3d_context_t* context;

  // none of these should throw; a crash fails the test binary regardless
  context = f3d_context_glx();
  if (context)
  {
    f3d_context_delete(context);
  }

  context = f3d_context_wgl();
  if (context)
  {
    f3d_context_delete(context);
  }

  context = f3d_context_cocoa();
  if (context)
  {
    f3d_context_delete(context);
  }

  context = f3d_context_egl();
  if (context)
  {
    f3d_context_delete(context);
  }

  context = f3d_context_osmesa();
  if (context)
  {
    f3d_context_delete(context);
  }

  return f3d_test_result(&test);
}
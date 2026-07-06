#include "pseudo_unit_test.h"

#include <context_c_api.h>

int test_context()
{
  f3d_test_t test;
  f3d_test_init(&test);
  
  f3d_context_t* context;

  // none of the functions should throw exception
  context = f3d_context_glx();
  f3d_test_check(&test, "glx context call did not throw", 1);
  if (context)
  {
    f3d_context_delete(context);
  }

  context = f3d_context_wgl();
  f3d_test_check(&test, "wgl context call did not throw", 1);
  if (context)
  {
    f3d_context_delete(context);
  }

  context = f3d_context_cocoa();
  f3d_test_check(&test, "cocoa context call did not throw", 1);
  if (context)
  {
    f3d_context_delete(context);
  }

  context = f3d_context_egl();
  f3d_test_check(&test, "egl context call did not throw", 1);
  if (context)
  {
    f3d_context_delete(context);
  }

  context = f3d_context_osmesa();
  f3d_test_check(&test, "osmesa context call did not throw", 1);
  if (context)
  {
    f3d_context_delete(context);
  }

  return f3d_test_result(&test);
}

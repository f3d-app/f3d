#include "pseudo_unit_test.h"

#include <context_c_api.h>

int test_context()
{
  f3d_test_t test;
  f3d_test_init(&test);

  f3d_context_t* context;

#ifdef __linux__
  context = f3d_context_glx();
  f3d_test_check(&test, "GLX context creation succeeds on Linux", context != NULL);
  if (context)
  {
    f3d_context_delete(context);
  }

  context = f3d_context_egl();
  f3d_test_check(&test, "EGL context creation succeeds on Linux", context != NULL);
  if (context)
  {
    f3d_context_delete(context);
  }
#endif

  /* wgl/cocoa are platform-specific to Windows/macOS; on Linux these are
   * expected to return NULL per the header docs, so we only check the call
   * completes without crashing, not a specific non-null result */
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

  /* osmesa is a software fallback that could plausibly work on any platform,
   * but its actual availability depends on whether OSMesa is available at
   * runtime */
  context = f3d_context_osmesa();
  if (context)
  {
    f3d_context_delete(context);
  }

  return f3d_test_result(&test);
}

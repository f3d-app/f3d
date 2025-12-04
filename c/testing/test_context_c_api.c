#include <context_c_api.h>

#include <stdio.h>

int test_context_c_api()
{
  int failed = 0;

#ifdef __linux__
  f3d_context_t* ctx = f3d_context_glx();
  if (ctx)
  {
    if (!f3d_context_get_function(ctx))
    {
      puts("[ERROR] GLX: failed to get function pointer");
      failed++;
    }
    f3d_context_delete(ctx);
  }

  ctx = f3d_context_egl();
  if (ctx)
  {
    if (!f3d_context_get_function(ctx))
    {
      puts("[ERROR] EGL: failed to get function pointer");
      failed++;
    }
    f3d_context_delete(ctx);
  }
#endif

#ifdef _WIN32
  f3d_context_t* ctx = f3d_context_wgl();
  if (ctx)
  {
    if (!f3d_context_get_function(ctx))
    {
      puts("[ERROR] WGL: failed to get function pointer");
      failed++;
    }
    f3d_context_delete(ctx);
  }
#endif

#ifdef __APPLE__
  f3d_context_t* ctx = f3d_context_cocoa();
  if (ctx)
  {
    if (!f3d_context_get_function(ctx))
    {
      puts("[ERROR] Cocoa: failed to get function pointer");
      failed++;
    }
    f3d_context_delete(ctx);
  }
#endif

  return failed;
}

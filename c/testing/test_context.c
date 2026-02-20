#include <context_c_api.h>

int test_context()
{
  f3d_context_t* context;

  // none of the functions should throw exception
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

  return 0;
}

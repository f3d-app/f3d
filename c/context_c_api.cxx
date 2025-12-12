#include "context_c_api.h"
#include "context.h"

struct f3d_context
{
  f3d::context::function func;

  explicit f3d_context(f3d::context::function&& f)
    : func(std::move(f))
  {
  }
};

//----------------------------------------------------------------------------
f3d_context_t* f3d_context_glx()
{
  return new f3d_context_t(f3d::context::glx());
}

//----------------------------------------------------------------------------
f3d_context_t* f3d_context_wgl()
{
  return new f3d_context_t(f3d::context::wgl());
}

//----------------------------------------------------------------------------
f3d_context_t* f3d_context_cocoa()
{
  return new f3d_context_t(f3d::context::cocoa());
}

//----------------------------------------------------------------------------
f3d_context_t* f3d_context_egl()
{
  return new f3d_context_t(f3d::context::egl());
}

//----------------------------------------------------------------------------
f3d_context_t* f3d_context_osmesa()
{
  return new f3d_context_t(f3d::context::osmesa());
}

//----------------------------------------------------------------------------
f3d_context_t* f3d_context_get_symbol(const char* lib, const char* func)
{
  if (!lib || !func)
  {
    return nullptr;
  }

  return new f3d_context_t(f3d::context::getSymbol(lib, func));
}

//----------------------------------------------------------------------------
void f3d_context_delete(f3d_context_t* ctx)
{
  delete ctx;
}

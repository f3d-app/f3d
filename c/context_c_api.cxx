#include "context_c_api.h"
#include "context.h"
#include "log.h"

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
  f3d_context_t* context = nullptr;
  try
  {
    context = new f3d_context_t(f3d::context::glx());
  }
  catch (f3d::context::loading_exception& e)
  {
    f3d::log::error(e.what());
    return nullptr;
  }
  catch (f3d::context::symbol_exception& e)
  {
    f3d::log::error(e.what());
    return nullptr;
  }

  return context;
}

//----------------------------------------------------------------------------
f3d_context_t* f3d_context_wgl()
{
  f3d_context_t* context = nullptr;
  try
  {
    context = new f3d_context_t(f3d::context::wgl());
  }
  catch (f3d::context::loading_exception& e)
  {
    f3d::log::error(e.what());
    return nullptr;
  }

  return context;
}

//----------------------------------------------------------------------------
f3d_context_t* f3d_context_cocoa()
{
  f3d_context_t* context = nullptr;
  try
  {
    context = new f3d_context_t(f3d::context::cocoa());
  }
  catch (f3d::context::loading_exception& e)
  {
    f3d::log::error(e.what());
    return nullptr;
  }

  return context;
}

//----------------------------------------------------------------------------
f3d_context_t* f3d_context_egl()
{
  f3d_context_t* context = nullptr;
  try
  {
    context = new f3d_context_t(f3d::context::egl());
  }
  catch (f3d::context::loading_exception& e)
  {
    f3d::log::error(e.what());
    return nullptr;
  }
  catch (f3d::context::symbol_exception& e)
  {
    f3d::log::error(e.what());
    return nullptr;
  }

  return context;
}

//----------------------------------------------------------------------------
f3d_context_t* f3d_context_osmesa()
{
  f3d_context_t* context = nullptr;
  try
  {
    context = new f3d_context_t(f3d::context::osmesa());
  }
  catch (f3d::context::loading_exception& e)
  {
    f3d::log::error(e.what());
    return nullptr;
  }
  catch (f3d::context::symbol_exception& e)
  {
    f3d::log::error(e.what());
    return nullptr;
  }

  return context;
}

//----------------------------------------------------------------------------
f3d_context_t* f3d_context_get_symbol(const char* lib, const char* func)
{
  if (!lib || !func)
  {
    return nullptr;
  }

  f3d_context_t* context = nullptr;
  try
  {
    context = new f3d_context_t(f3d::context::getSymbol(lib, func));
  } catch (f3d::context::loading_exception& e)
  {
    f3d::log::error(e.what());
    return nullptr;
  }
  catch (f3d::context::symbol_exception& e)
  {
    f3d::log::error(e.what());
    return nullptr;
  }

  return context;
}

//----------------------------------------------------------------------------
void f3d_context_delete(f3d_context_t* ctx)
{
  delete ctx;
}

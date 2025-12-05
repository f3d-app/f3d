#include "engine_c_api.h"
#include "engine.h"

//----------------------------------------------------------------------------
f3d_engine_t* f3d_engine_create(int offscreen)
{
  f3d::engine* eng = new f3d::engine(f3d::engine::create(offscreen != 0));
  return reinterpret_cast<f3d_engine_t*>(eng);
}

//----------------------------------------------------------------------------
f3d_engine_t* f3d_engine_create_none()
{
  f3d::engine* eng = new f3d::engine(f3d::engine::createNone());
  return reinterpret_cast<f3d_engine_t*>(eng);
}

//----------------------------------------------------------------------------
f3d_engine_t* f3d_engine_create_glx(int offscreen)
{
  f3d::engine* eng = new f3d::engine(f3d::engine::createGLX(offscreen != 0));
  return reinterpret_cast<f3d_engine_t*>(eng);
}

//----------------------------------------------------------------------------
f3d_engine_t* f3d_engine_create_wgl(int offscreen)
{
  f3d::engine* eng = new f3d::engine(f3d::engine::createWGL(offscreen != 0));
  return reinterpret_cast<f3d_engine_t*>(eng);
}

//----------------------------------------------------------------------------
f3d_engine_t* f3d_engine_create_egl()
{
  f3d::engine* eng = new f3d::engine(f3d::engine::createEGL());
  return reinterpret_cast<f3d_engine_t*>(eng);
}

//----------------------------------------------------------------------------
f3d_engine_t* f3d_engine_create_osmesa()
{
  f3d::engine* eng = new f3d::engine(f3d::engine::createOSMesa());
  return reinterpret_cast<f3d_engine_t*>(eng);
}

//----------------------------------------------------------------------------
f3d_engine_t* f3d_engine_create_external_glx()
{
  f3d::engine* eng = new f3d::engine(f3d::engine::createExternalGLX());
  return reinterpret_cast<f3d_engine_t*>(eng);
}

//----------------------------------------------------------------------------
f3d_engine_t* f3d_engine_create_external_wgl()
{
  f3d::engine* eng = new f3d::engine(f3d::engine::createExternalWGL());
  return reinterpret_cast<f3d_engine_t*>(eng);
}

//----------------------------------------------------------------------------
f3d_engine_t* f3d_engine_create_external_cocoa()
{
  f3d::engine* eng = new f3d::engine(f3d::engine::createExternalCOCOA());
  return reinterpret_cast<f3d_engine_t*>(eng);
}

//----------------------------------------------------------------------------
f3d_engine_t* f3d_engine_create_external_egl()
{
  f3d::engine* eng = new f3d::engine(f3d::engine::createExternalEGL());
  return reinterpret_cast<f3d_engine_t*>(eng);
}

//----------------------------------------------------------------------------
f3d_engine_t* f3d_engine_create_external_osmesa()
{
  f3d::engine* eng = new f3d::engine(f3d::engine::createExternalOSMesa());
  return reinterpret_cast<f3d_engine_t*>(eng);
}

//----------------------------------------------------------------------------
void f3d_engine_delete(f3d_engine_t* engine)
{
  if (!engine)
  {
    return;
  }

  f3d::engine* cpp_engine = reinterpret_cast<f3d::engine*>(engine);
  delete cpp_engine;
}

//----------------------------------------------------------------------------
void f3d_engine_set_cache_path(f3d_engine_t* engine, const char* cache_path)
{
  if (!engine || !cache_path)
  {
    return;
  }

  f3d::engine* cpp_engine = reinterpret_cast<f3d::engine*>(engine);
  cpp_engine->setCachePath(cache_path);
}

//----------------------------------------------------------------------------
f3d_options_t* f3d_engine_get_options(f3d_engine_t* engine)
{
  if (!engine)
  {
    return nullptr;
  }

  f3d::engine* cpp_engine = reinterpret_cast<f3d::engine*>(engine);
  f3d::options& opts = cpp_engine->getOptions();
  return reinterpret_cast<f3d_options_t*>(&opts);
}

//----------------------------------------------------------------------------
f3d_window_t* f3d_engine_get_window(f3d_engine_t* engine)
{
  if (!engine)
  {
    return nullptr;
  }

  f3d::engine* cpp_engine = reinterpret_cast<f3d::engine*>(engine);
  f3d::window& win = cpp_engine->getWindow();
  return reinterpret_cast<f3d_window_t*>(&win);
}

//----------------------------------------------------------------------------
f3d_scene_t* f3d_engine_get_scene(f3d_engine_t* engine)
{
  if (!engine)
  {
    return nullptr;
  }

  f3d::engine* cpp_engine = reinterpret_cast<f3d::engine*>(engine);
  f3d::scene& sce = cpp_engine->getScene();
  return reinterpret_cast<f3d_scene_t*>(&sce);
}

//----------------------------------------------------------------------------
f3d_interactor_t* f3d_engine_get_interactor(f3d_engine_t* engine)
{
  if (!engine)
  {
    return nullptr;
  }

  f3d::engine* cpp_engine = reinterpret_cast<f3d::engine*>(engine);
  f3d::interactor& inter = cpp_engine->getInteractor();
  return reinterpret_cast<f3d_interactor_t*>(&inter);
}

//----------------------------------------------------------------------------
void f3d_engine_load_plugin(const char* path_or_name)
{
  if (!path_or_name)
  {
    return;
  }

  f3d::engine::loadPlugin(path_or_name);
}

//----------------------------------------------------------------------------
void f3d_engine_autoload_plugins()
{
  f3d::engine::autoloadPlugins();
}

//----------------------------------------------------------------------------
void f3d_engine_set_reader_option(const char* name, const char* value)
{
  if (!name || !value)
  {
    return;
  }

  f3d::engine::setReaderOption(name, value);
}

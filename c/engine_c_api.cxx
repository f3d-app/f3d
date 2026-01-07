#include "engine_c_api.h"
#include "engine.h"
#include "options.h"

#include <cstring>

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
f3d_engine_t* f3d_engine_create_external(f3d_context_function_t get_proc_address)
{
  if (!get_proc_address)
  {
    return nullptr;
  }

  f3d::context::function func = [get_proc_address](const char* name) -> f3d::context::fptr
  { return get_proc_address(name); };

  f3d::engine* eng = new f3d::engine(f3d::engine::createExternal(func));
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
void f3d_engine_set_options(f3d_engine_t* engine, f3d_options_t* options)
{
  if (!engine || !options)
  {
    return;
  }

  f3d::engine* cpp_engine = reinterpret_cast<f3d::engine*>(engine);
  f3d::options* cpp_options = reinterpret_cast<f3d::options*>(options);
  cpp_engine->setOptions(*cpp_options);
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
char** f3d_engine_get_plugins_list(const char* plugin_path)
{
  if (!plugin_path)
  {
    return nullptr;
  }

  std::vector<std::string> plugins = f3d::engine::getPluginsList(plugin_path);

  if (plugins.empty())
  {
    return nullptr;
  }

  char** result = new char*[plugins.size() + 1];

  for (size_t i = 0; i < plugins.size(); i++)
  {
    result[i] = new char[plugins[i].length() + 1];
    std::strcpy(result[i], plugins[i].c_str());
  }

  result[plugins.size()] = nullptr;
  return result;
}

//----------------------------------------------------------------------------
char** f3d_engine_get_all_reader_option_names()
{
  std::vector<std::string> names = f3d::engine::getAllReaderOptionNames();

  char** result = new char*[names.size() + 1];

  for (size_t i = 0; i < names.size(); i++)
  {
    result[i] = new char[names[i].length() + 1];
    std::strcpy(result[i], names[i].c_str());
  }

  result[names.size()] = nullptr;
  return result;
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

//----------------------------------------------------------------------------
f3d_backend_info_t* f3d_engine_get_rendering_backend_list(int* count)
{
  std::map<std::string, bool> backends = f3d::engine::getRenderingBackendList();

  f3d_backend_info_t* result = new f3d_backend_info_t[backends.size() + 1];

  size_t i = 0;
  for (const auto& [name, available] : backends)
  {
    result[i].name = new char[name.length() + 1];
    std::strcpy(result[i].name, name.c_str());
    result[i].available = available ? 1 : 0;
    i++;
  }

  result[backends.size()].name = nullptr;
  result[backends.size()].available = 0;

  if (count)
  {
    *count = static_cast<int>(backends.size());
  }

  return result;
}

//----------------------------------------------------------------------------
void f3d_engine_free_backend_list(f3d_backend_info_t* backends)
{
  if (!backends)
  {
    return;
  }

  for (int i = 0; backends[i].name != nullptr; i++)
  {
    delete[] backends[i].name;
  }

  delete[] backends;
}

//----------------------------------------------------------------------------
void f3d_engine_free_string_array(char** array)
{
  if (!array)
  {
    return;
  }

  for (int i = 0; array[i] != nullptr; i++)
  {
    delete[] array[i];
  }

  delete[] array;
}

//----------------------------------------------------------------------------
f3d_lib_info_t* f3d_engine_get_lib_info()
{
  f3d::engine::libInformation cpp_info = f3d::engine::getLibInfo();

  f3d_lib_info_t* info = new f3d_lib_info_t;

  info->version = new char[cpp_info.Version.length() + 1];
  std::strcpy(info->version, cpp_info.Version.c_str());

  info->version_full = new char[cpp_info.VersionFull.length() + 1];
  std::strcpy(info->version_full, cpp_info.VersionFull.c_str());

  info->build_date = new char[cpp_info.BuildDate.length() + 1];
  std::strcpy(info->build_date, cpp_info.BuildDate.c_str());

  info->build_system = new char[cpp_info.BuildSystem.length() + 1];
  std::strcpy(info->build_system, cpp_info.BuildSystem.c_str());

  info->compiler = new char[cpp_info.Compiler.length() + 1];
  std::strcpy(info->compiler, cpp_info.Compiler.c_str());

  info->vtk_version = new char[cpp_info.VTKVersion.length() + 1];
  std::strcpy(info->vtk_version, cpp_info.VTKVersion.c_str());

  info->license = new char[cpp_info.License.length() + 1];
  std::strcpy(info->license, cpp_info.License.c_str());

  info->modules = new f3d_module_info_t[cpp_info.Modules.size() + 1];
  size_t i = 0;
  for (const auto& [name, available] : cpp_info.Modules)
  {
    info->modules[i].name = new char[name.length() + 1];
    std::strcpy(info->modules[i].name, name.c_str());
    info->modules[i].available = available ? 1 : 0;
    i++;
  }
  info->modules[cpp_info.Modules.size()].name = nullptr;
  info->modules[cpp_info.Modules.size()].available = 0;

  info->copyrights = new char*[cpp_info.Copyrights.size() + 1];
  for (size_t j = 0; j < cpp_info.Copyrights.size(); j++)
  {
    info->copyrights[j] = new char[cpp_info.Copyrights[j].length() + 1];
    std::strcpy(info->copyrights[j], cpp_info.Copyrights[j].c_str());
  }
  info->copyrights[cpp_info.Copyrights.size()] = nullptr;

  return info;
}

//----------------------------------------------------------------------------
void f3d_engine_free_lib_info(f3d_lib_info_t* info)
{
  if (!info)
  {
    return;
  }

  delete[] info->version;
  delete[] info->version_full;
  delete[] info->build_date;
  delete[] info->build_system;
  delete[] info->compiler;
  delete[] info->vtk_version;
  delete[] info->license;

  if (info->modules)
  {
    for (int i = 0; info->modules[i].name != nullptr; i++)
    {
      delete[] info->modules[i].name;
    }
    delete[] info->modules;
  }

  if (info->copyrights)
  {
    for (int i = 0; info->copyrights[i] != nullptr; i++)
    {
      delete[] info->copyrights[i];
    }
    delete[] info->copyrights;
  }

  delete info;
}

//----------------------------------------------------------------------------
f3d_reader_info_t* f3d_engine_get_readers_info(int* count)
{
  std::vector<f3d::engine::readerInformation> cpp_readers = f3d::engine::getReadersInfo();

  f3d_reader_info_t* readers = new f3d_reader_info_t[cpp_readers.size() + 1];

  for (size_t i = 0; i < cpp_readers.size(); i++)
  {
    const auto& cpp_reader = cpp_readers[i];

    readers[i].name = new char[cpp_reader.Name.length() + 1];
    std::strcpy(readers[i].name, cpp_reader.Name.c_str());

    readers[i].description = new char[cpp_reader.Description.length() + 1];
    std::strcpy(readers[i].description, cpp_reader.Description.c_str());

    readers[i].plugin_name = new char[cpp_reader.PluginName.length() + 1];
    std::strcpy(readers[i].plugin_name, cpp_reader.PluginName.c_str());

    readers[i].extensions = new char*[cpp_reader.Extensions.size() + 1];
    for (size_t j = 0; j < cpp_reader.Extensions.size(); j++)
    {
      readers[i].extensions[j] = new char[cpp_reader.Extensions[j].length() + 1];
      std::strcpy(readers[i].extensions[j], cpp_reader.Extensions[j].c_str());
    }
    readers[i].extensions[cpp_reader.Extensions.size()] = nullptr;

    readers[i].mime_types = new char*[cpp_reader.MimeTypes.size() + 1];
    for (size_t j = 0; j < cpp_reader.MimeTypes.size(); j++)
    {
      readers[i].mime_types[j] = new char[cpp_reader.MimeTypes[j].length() + 1];
      std::strcpy(readers[i].mime_types[j], cpp_reader.MimeTypes[j].c_str());
    }
    readers[i].mime_types[cpp_reader.MimeTypes.size()] = nullptr;

    readers[i].has_scene_reader = cpp_reader.HasSceneReader ? 1 : 0;
    readers[i].has_geometry_reader = cpp_reader.HasGeometryReader ? 1 : 0;
  }

  readers[cpp_readers.size()].name = nullptr;
  readers[cpp_readers.size()].description = nullptr;
  readers[cpp_readers.size()].extensions = nullptr;
  readers[cpp_readers.size()].mime_types = nullptr;
  readers[cpp_readers.size()].plugin_name = nullptr;
  readers[cpp_readers.size()].has_scene_reader = 0;
  readers[cpp_readers.size()].has_geometry_reader = 0;

  if (count)
  {
    *count = static_cast<int>(cpp_readers.size());
  }

  return readers;
}

//----------------------------------------------------------------------------
void f3d_engine_free_readers_info(f3d_reader_info_t* readers)
{
  if (!readers)
  {
    return;
  }

  for (int i = 0; readers[i].name != nullptr; i++)
  {
    delete[] readers[i].name;
    delete[] readers[i].description;
    delete[] readers[i].plugin_name;

    if (readers[i].extensions)
    {
      for (int j = 0; readers[i].extensions[j] != nullptr; j++)
      {
        delete[] readers[i].extensions[j];
      }
      delete[] readers[i].extensions;
    }

    if (readers[i].mime_types)
    {
      for (int j = 0; readers[i].mime_types[j] != nullptr; j++)
      {
        delete[] readers[i].mime_types[j];
      }
      delete[] readers[i].mime_types;
    }
  }

  delete[] readers;
}

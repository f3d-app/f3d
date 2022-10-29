#include "options.h"

#include "log.h"

#include "vtkF3DConfigure.h"

#include <map>
#include <type_traits>
#include <variant>

namespace f3d
{
//----------------------------------------------------------------------------
class options::internals
{
public:
  using OptionVariant =
    std::variant<bool, int, double, std::string, std::vector<int>, std::vector<double> >;

  template<typename T, typename U>
  struct IsTypeValid;

  template<typename T, typename... Ts>
  struct IsTypeValid<T, std::variant<Ts...> > : public std::disjunction<std::is_same<T, Ts>...>
  {
  };

  template<typename T>
  void init(const std::string& name, const T& value)
  {
    static_assert(IsTypeValid<T, OptionVariant>::value);
    this->Options[name] = value;
  }

  template<typename T>
  void set(const std::string& name, const T& value)
  {
    static_assert(!std::is_array_v<T> && !std::is_pointer_v<T>);
    try
    {
      T& opt = std::get<T>(this->Options.at(name));
      opt = value;
    }
    catch (const std::bad_variant_access&)
    {
      log::error("Trying to set option ", name, " with incompatible type");
    }
    catch (const std::out_of_range&)
    {
      log::error("Options ", name, " does not exist");
    }
  }

  template<typename T>
  void get(const std::string& name, T& value) const
  {
    try
    {
      value = std::get<T>(this->Options.at(name));
    }
    catch (const std::bad_variant_access&)
    {
      log::error("Trying to get option ", name, " with incompatible type");
      return;
    }
    catch (const std::out_of_range&)
    {
      log::error("Options ", name, " does not exist");
      return;
    }
  }

  template<typename T>
  T get(const std::string& name) const
  {
    T val = {};
    this->get(name, val);
    return val;
  }

  template<typename T>
  T& getRef(const std::string& name)
  {
    try
    {
      return std::get<T>(this->Options.at(name));
    }
    catch (const std::bad_variant_access&)
    {
      std::string error = "Trying to get option reference " + name + " with incompatible type";
      log::error(error);
      throw options::incompatible_exception(error + "\n");
    }
    catch (const std::out_of_range&)
    {
      std::string error = "Options " + name + " does not exist";
      log::error(error);
      throw options::inexistent_exception(error + "\n");
    }
  }

  std::map<std::string, OptionVariant> Options;
};

//----------------------------------------------------------------------------
options::options()
  : Internals(new options::internals)
{
  // Scene
  this->Internals->init("scene.animation.index", 0);
  this->Internals->init("scene.camera.index", -1);
  this->Internals->init("scene.geometry-only", false);
  this->Internals->init("scene.up-direction", std::string("+Y"));

  // Render
  this->Internals->init("render.show-edges", false);
  this->Internals->init("render.line-width", 1.0);
  this->Internals->init("render.point-size", 10.0);
  this->Internals->init("render.grid", false);

  this->Internals->init("render.raytracing.enable", false);
  this->Internals->init("render.raytracing.denoise", false);
  this->Internals->init("render.raytracing.samples", 5);

  this->Internals->init("render.effect.depth-peeling", false);
  this->Internals->init("render.effect.fxaa", false);
  this->Internals->init("render.effect.ssao", false);
  this->Internals->init("render.effect.tone-mapping", false);

  this->Internals->init("render.background.color", std::vector<double>{ 0.2, 0.2, 0.2 });
  this->Internals->init(
    "render.background.hdri", std::string()); // XXX This overrides background.color
  this->Internals->init("render.background.blur", false);
  this->Internals->init("render.light.intensity", 1.);

  // UI
  this->Internals->init("ui.bar", false);
  this->Internals->init("ui.filename", false);
  this->Internals->init("ui.fps", false);
  this->Internals->init("ui.cheatsheet", false);
  this->Internals->init("ui.metadata", false);
  this->Internals->init("ui.font-file", std::string());
  this->Internals->init("ui.loader-progress", false);

  // Model
  this->Internals->init("model.color.opacity", 1.0);
  // XXX: Not compatible with scivis: https://github.com/f3d-app/f3d/issues/347
  this->Internals->init("model.color.rgb", std::vector<double>{ 1., 1., 1. });
  // XXX: Artifacts when using with scivis: https://github.com/f3d-app/f3d/issues/348
  this->Internals->init("model.color.texture", std::string());

  this->Internals->init("model.emissive.factor", std::vector<double>{ 1., 1., 1. });
  this->Internals->init("model.emissive.texture", std::string());

  this->Internals->init("model.normal.texture", std::string());
  this->Internals->init("model.normal.scale", 1.0);

  this->Internals->init("model.material.metallic", 0.0);
  this->Internals->init("model.material.roughness", 0.3);
  this->Internals->init("model.material.texture", std::string());

  this->Internals->init("model.scivis.cells", false);
  this->Internals->init("model.scivis.array-name", F3D_RESERVED_STRING);
  this->Internals->init("model.scivis.component", -1);
  this->Internals->init("model.scivis.colormap",
    std::vector<double>{
      0.0, 0.0, 0.0, 0.0, 0.4, 0.9, 0.0, 0.0, 0.8, 0.9, 0.9, 0.0, 1.0, 1.0, 1.0, 1.0 });
  this->Internals->init("model.scivis.range", std::vector<double>{ 0 });

  // XXX: Rename into a "rendering-mode" option: https://github.com/f3d-app/f3d/issues/345
  this->Internals->init("model.point-sprites.enable", false);
  this->Internals->init("model.volume.enable", false);
  this->Internals->init("model.volume.inverse", false);

  // Interactor
  this->Internals->init("interactor.axis", false);
  this->Internals->init("interactor.trackball", false);
};

//----------------------------------------------------------------------------
options::~options()
{
  delete this->Internals;
}

//----------------------------------------------------------------------------
options::options(const options& opt)
  : Internals(new options::internals)
{
  this->Internals->Options = opt.Internals->Options;
}

//----------------------------------------------------------------------------
options& options::operator=(const options& opt) noexcept
{
  this->Internals->Options = opt.Internals->Options;
  return *this;
}

//----------------------------------------------------------------------------
options::options(options&& other) noexcept
{
  this->Internals = other.Internals;
  other.Internals = nullptr;
}

//----------------------------------------------------------------------------
options& options::operator=(options&& other) noexcept
{
  if (this != &other)
  {
    delete this->Internals;
    this->Internals = other.Internals;
    other.Internals = nullptr;
  }
  return *this;
}

//----------------------------------------------------------------------------
options& options::set(const std::string& name, bool value)
{
  this->Internals->set(name, value);
  return *this;
}

//----------------------------------------------------------------------------
options& options::set(const std::string& name, int value)
{
  this->Internals->set(name, value);
  return *this;
}

//----------------------------------------------------------------------------
options& options::set(const std::string& name, double value)
{
  this->Internals->set(name, value);
  return *this;
}

//----------------------------------------------------------------------------
options& options::set(const std::string& name, const std::string& value)
{
  this->Internals->set(name, value);
  return *this;
}

//----------------------------------------------------------------------------
options& options::set(const std::string& name, const char* value)
{
  this->Internals->set(name, std::string(value));
  return *this;
}

//----------------------------------------------------------------------------
options& options::set(const std::string& name, const std::vector<int>& values)
{
  this->Internals->set(name, values);
  return *this;
}

//----------------------------------------------------------------------------
options& options::set(const std::string& name, const std::vector<double>& values)
{
  this->Internals->set(name, values);
  return *this;
}

//----------------------------------------------------------------------------
options& options::set(const std::string& name, std::initializer_list<int> values)
{
  this->Internals->set(name, std::vector<int>(values));
  return *this;
}

//----------------------------------------------------------------------------
options& options::set(const std::string& name, std::initializer_list<double> values)
{
  this->Internals->set(name, std::vector<double>(values));
  return *this;
}

//----------------------------------------------------------------------------
void options::get(const std::string& name, bool& value) const
{
  this->Internals->get(name, value);
}

//----------------------------------------------------------------------------
void options::get(const std::string& name, int& value) const
{
  this->Internals->get(name, value);
}

//----------------------------------------------------------------------------
void options::get(const std::string& name, double& value) const
{
  this->Internals->get(name, value);
}

//----------------------------------------------------------------------------
void options::get(const std::string& name, std::string& value) const
{
  this->Internals->get(name, value);
}

//----------------------------------------------------------------------------
void options::get(const std::string& name, std::vector<int>& value) const
{
  this->Internals->get(name, value);
}

//----------------------------------------------------------------------------
void options::get(const std::string& name, std::vector<double>& value) const
{
  this->Internals->get(name, value);
}

//----------------------------------------------------------------------------
bool options::getAsBool(const std::string& name) const
{
  return this->Internals->get<bool>(name);
}

//----------------------------------------------------------------------------
int options::getAsInt(const std::string& name) const
{
  return this->Internals->get<int>(name);
}

//----------------------------------------------------------------------------
double options::getAsDouble(const std::string& name) const
{
  return this->Internals->get<double>(name);
}

//----------------------------------------------------------------------------
std::string options::getAsString(const std::string& name) const
{
  return this->Internals->get<std::string>(name);
}

//----------------------------------------------------------------------------
std::vector<int> options::getAsIntVector(const std::string& name) const
{
  return this->Internals->get<std::vector<int> >(name);
}

//----------------------------------------------------------------------------
std::vector<double> options::getAsDoubleVector(const std::string& name) const
{
  return this->Internals->get<std::vector<double> >(name);
}

//----------------------------------------------------------------------------
bool& options::getAsBoolRef(const std::string& name)
{
  return this->Internals->getRef<bool>(name);
}

//----------------------------------------------------------------------------
int& options::getAsIntRef(const std::string& name)
{
  return this->Internals->getRef<int>(name);
}

//----------------------------------------------------------------------------
double& options::getAsDoubleRef(const std::string& name)
{
  return this->Internals->getRef<double>(name);
}

//----------------------------------------------------------------------------
std::string& options::getAsStringRef(const std::string& name)
{
  return this->Internals->getRef<std::string>(name);
}

//----------------------------------------------------------------------------
std::vector<int>& options::getAsIntVectorRef(const std::string& name)
{
  return this->Internals->getRef<std::vector<int> >(name);
}

//----------------------------------------------------------------------------
std::vector<double>& options::getAsDoubleVectorRef(const std::string& name)
{
  return this->Internals->getRef<std::vector<double> >(name);
}

//----------------------------------------------------------------------------
options& options::toggle(const std::string& name)
{
  this->Internals->set<bool>(name, !this->Internals->get<bool>(name));
  return *this;
}

//----------------------------------------------------------------------------
bool options::isSame(const options& other, const std::string& name) const
{
  try
  {
    return this->Internals->Options.at(name) == other.Internals->Options.at(name);
  }
  catch (const std::out_of_range&)
  {
    std::string error = "Options " + name + " does not exist";
    log::error(error);
    throw options::inexistent_exception(error + "\n");
  }
}

//----------------------------------------------------------------------------
options& options::copy(const options& from, const std::string& name)
{
  try
  {
    this->Internals->Options.at(name) = from.Internals->Options.at(name);
  }
  catch (const std::out_of_range&)
  {
    std::string error = "Options " + name + " does not exist";
    log::error(error);
    throw options::inexistent_exception(error + "\n");
  }
  return *this;
}

//----------------------------------------------------------------------------
std::vector<std::string> options::getNames()
{
  std::vector<std::string> names;
  for (const auto& [name, _] : this->Internals->Options)
  {
    names.emplace_back(name);
  }
  return names;
}

//----------------------------------------------------------------------------
options::incompatible_exception::incompatible_exception(const std::string& what)
  : exception(what)
{
}

//----------------------------------------------------------------------------
options::inexistent_exception::inexistent_exception(const std::string& what)
  : exception(what)
{
}

}

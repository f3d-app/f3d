#include "options.h"

#include "export.h"
#include "init.h"
#include "log.h"
#include "utils.h"

#include "vtkF3DConfigure.h"

#include <algorithm>
#include <limits>
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
#ifndef F3D_NO_DEPRECATED
      if (this->IsDeprecated(name))
      {
        log::warn("Option ", name, " is deprecated");
      }
#endif
      OptionVariant var(value);
      if (name == "scene.animation.index")
      {
        this->option_struct.scene.animation.index = std::get<int>(var);
      }
      else if (name == "render.line_width")
      {
        this->option_struct.render.line_width = std::get<double>(var);
      }
    }
    catch (const std::bad_variant_access&)
    {
      log::error("Trying to set option ", name, " with incompatible type");
    }
/*    catch (const std::out_of_range&)
    {
      log::error("Option ", name, " does not exist");
    }*/
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
      log::error("Option ", name, " does not exist");
      return;
    }
  }

  template<typename T>
  T get(const std::string& name) const
  {
    OptionVariant var;
    T val = {};
    try
    {
      if (name == "scene.animation.index")
      {
        var = this->option_struct.scene.animation.index;
      }
      else if (name == "render.line_width")
      {
        var = this->option_struct.render.line_width;
      }
      T val = std::get<T>(var);
    }
    catch (const std::bad_variant_access&)
    {
      log::error("Trying to get option ", name, " with incompatible type");
    }
    return val;
  }

  template<typename T>
  T& getRef(const std::string& name)
  {
    try
    {
#ifndef F3D_NO_DEPRECATED
      if (this->IsDeprecated(name))
      {
        log::warn("Option ", name, " is deprecated");
      }
#endif
      return std::get<T>(this->Options.at(name));
    }
    catch (const std::bad_variant_access&)
    {
      throw options::incompatible_exception(
        "Trying to get option reference " + name + " with incompatible type");
    }
    catch (const std::out_of_range&)
    {
      throw options::inexistent_exception("Option " + name + " does not exist");
    }
  }

#ifndef F3D_NO_DEPRECATED
  bool IsDeprecated(const std::string& name)
  {
    // compile time list of deprecated options
    constexpr std::string_view deprecated[] = { "render.background.hdri" };

    auto it = std::find(std::begin(deprecated), std::end(deprecated), name);
    return it != std::end(deprecated);
  }
#endif

  void setVariant(const std::string& name, variant_t value)
  {
    if (name == "scene.animation.index")
    {
      this->option_struct.scene.animation.index = std::get<int>(value);
    }
    else if (name == "render.line_width")
    {
      this->option_struct.render.line_width = std::get<double>(value);
    }
  }

  variant_t getVariant(const std::string& name)
  {
    variant_t var;
    if (name == "scene.animation.index")
    {
      var = this->option_struct.scene.animation.index;
    }
    else if (name == "render.line_width")
    {
      var = this->option_struct.render.line_width;
    }
    return var;
  }

  void setString(const std::string& name, std::string value)
  {
    variant_t var;
    if (name == "scene.animation.index")
    {
      var = std::stoi(value);
      this->setVariant(name, var);
    }
    else if (name == "render.line_width")
    {
      var = std::stof(value);
      this->setVariant(name, var);
    }
  }

  std::string getString(const std::string& name)
  {
    variant_t var = this->getVariant(name);
    std::string str;
    if (name == "scene.animation.index")
    {
      str = std::to_string(std::get<int>(var));
    }
    else if (name == "render.line_width")
    {
      str = std::to_string(std::get<double>(var));
    }
    return str;
  }

  std::map<std::string, OptionVariant> Options;
  f3d_options option_struct;
};

//----------------------------------------------------------------------------
options::options()
  : Internals(new options::internals)
{
  detail::init::initialize();

  // Scene
  this->Internals->init("scene.animation.autoplay", false);
  this->Internals->init("scene.animation.index", 0);
  this->Internals->init("scene.animation.speed-factor", 1.0);
  this->Internals->init("scene.animation.time", 0.0);
  this->Internals->init("scene.animation.frame-rate", 60.0);
  this->Internals->init("scene.camera.index", -1);
  this->Internals->init("scene.up-direction", std::string("+Y"));

  // Render
  this->Internals->init("render.show-edges", false);
  this->Internals->init("render.line-width", 1.0);
  this->Internals->init("render.point-size", 10.0);
  this->Internals->init("render.grid.enable", false);
  this->Internals->init("render.grid.absolute", false);
  this->Internals->init("render.grid.unit", 0.0);
  this->Internals->init("render.grid.subdivisions", 10);

  this->Internals->init("render.raytracing.enable", false);
  this->Internals->init("render.raytracing.denoise", false);
  this->Internals->init("render.raytracing.samples", 5);

  this->Internals->init("render.effect.translucency-support", false);
  this->Internals->init("render.effect.anti-aliasing", false);
  this->Internals->init("render.effect.ambient-occlusion", false);
  this->Internals->init("render.effect.tone-mapping", false);

  this->Internals->init("render.hdri.file", std::string());
  this->Internals->init("render.hdri.ambient", false);
  this->Internals->init("render.background.color", std::vector<double>{ 0.2, 0.2, 0.2 });
#ifndef F3D_NO_DEPRECATED
  this->Internals->init("render.background.hdri", std::string());
#endif
  this->Internals->init("render.background.skybox", false);
  this->Internals->init("render.background.blur", false);
  this->Internals->init("render.background.blur.coc", 20.0);
  this->Internals->init("render.light.intensity", 1.);

  // UI
  this->Internals->init("ui.bar", false);
  this->Internals->init("ui.filename", false);
  this->Internals->init("ui.filename-info", std::string());
  this->Internals->init("ui.fps", false);
  this->Internals->init("ui.cheatsheet", false);
  this->Internals->init("ui.dropzone", false);
  this->Internals->init("ui.dropzone-info", std::string());
  this->Internals->init("ui.metadata", false);
  this->Internals->init("ui.font-file", std::string());
  this->Internals->init("ui.loader-progress", false);

  // Model
  this->Internals->init("model.matcap.texture", std::string());

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
  this->Internals->init("model.point-sprites.type", std::string("sphere"));
  this->Internals->init("model.volume.enable", false);
  this->Internals->init("model.volume.inverse", false);

  // Camera projection
  this->Internals->init("scene.camera.orthographic", false);

  // Interactor
  this->Internals->init("interactor.axis", false);
  this->Internals->init("interactor.trackball", false);
  this->Internals->init("interactor.invert-zoom", false);
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
void options::setVariant(const std::string& name, variant_t value)
{
  this->Internals->setVariant(name, value);
}

//----------------------------------------------------------------------------
variant_t options::getVariant(const std::string& name)
{
  return this->Internals->getVariant(name);
}

//----------------------------------------------------------------------------
void options::setString(const std::string& name, std::string value)
{
  this->Internals->setString(name, value);
}

//----------------------------------------------------------------------------
std::string options::getString(const std::string& name)
{
  return this->Internals->getString(name);
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
  names.reserve(this->Internals->Options.size());
  for (const auto& [name, value] : this->Internals->Options)
  {
    names.emplace_back(name);
  }
  return names;
}

//----------------------------------------------------------------------------
std::pair<std::string, unsigned int> options::getClosestOption(const std::string& option) const
{
  if (this->Internals->Options.find(option) != this->Internals->Options.end())
  {
    return { option, 0 };
  }

  std::pair<std::string, int> ret = { "", std::numeric_limits<int>::max() };

  for (const auto& [name, value] : this->Internals->Options)
  {
    int distance = utils::textDistance(name, option);
    if (distance < ret.second)
    {
      ret = { name, distance };
    }
  }

  return ret;
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

f3d_options& options::getStruct(){return this->Internals->option_struct;}
const f3d_options& options::getConstStruct() const{return this->Internals->option_struct;}

}

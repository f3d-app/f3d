#include "f3d_options.h"

#include "F3DConfig.h"

#include "F3DLog.h"

#include <map>
#include <variant>

namespace f3d
{
//----------------------------------------------------------------------------
class options::F3DInternals
{
public:
  template<typename T>
  void set(const std::string& name, const T& value)
  {
    if (this->CheckExists)
    {
      try
      {
        T& opt = std::get<T>(this->Options.at(name));
        opt = value;
      }
      catch (std::bad_variant_access const&)
      {
        F3DLog::Print(
          F3DLog::Severity::Error, "Trying to set option ", name, " with incompatible type");
        return;
      }
      catch (const std::out_of_range&)
      {
        F3DLog::Print(F3DLog::Severity::Error, "Options ", name, " does not exist");
        return;
      }
    }
    else
    {
      this->Options[name] = value;
    }
  }

  template<typename T>
  void get(const std::string& name, T& value) const
  {
    try
    {
      const T& opt = std::get<T>(this->Options.at(name));
      value = opt;
    }
    catch (const std::bad_variant_access&)
    {
      F3DLog::Print(
        F3DLog::Severity::Error, "Trying to get option ", name, " with incompatible type");
      return;
    }
    catch (const std::out_of_range&)
    {
      F3DLog::Print(F3DLog::Severity::Error, "Options ", name, " does not exist");
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

  std::map<std::string,
    std::variant<bool, int, double, std::string, std::vector<int>, std::vector<double> > >
    Options;
  bool CheckExists = false;
};

//----------------------------------------------------------------------------
options::options()
  : Internals(new options::F3DInternals)
{
  // General
  this->set("quiet", false);
  this->set("verbose", false);

  // Loader/Loading
  this->set("animation-index", 0);
  this->set("geometry-only", false);
  this->set("progress", false);
  this->set("camera-index", 0);
  this->set("color", { 1., 1., 1. });
  this->set("emissive-factor", { 1., 1., 1. });
  this->set("line-width", 1.0);
  this->set("metallic", 0.0);
  this->set("normal-scale", 1.0);
  this->set("opacity", 1.0);
  this->set("point-size", 10.0);
  this->set("roughness", 0.3);
  this->set("texture-base-color", "");
  this->set("texture-emissive", "");
  this->set("texture-material", "");
  this->set("texture-normal", "");

  // Loading but should not
  this->set("cells", false);
  this->set("scalars", F3DReservedString);
  this->set("component", -1);
  this->set("fullscreen", false);
  this->set("resolution", { 1000, 600 });
  this->set("hdri", "");
  this->set("background-color", { 0.2, 0.2, 0.2 });
  this->set("up", "+Y");
  this->set("font-file", "");

  // Rendering/Dynamic
  this->set("axis", false);
  this->set("bar", false);
  this->set("blur-background", false);
  this->set("camera-azimuth-angle", 0.0);
  this->set("camera-elevation-angle", 0.0);
  this->set("camera-focal-point", std::vector<double>());
  this->set("camera-position", std::vector<double>());
  this->set("camera-view-angle", 0.0);
  this->set("camera-view-up", std::vector<double>());
  this->set(
    "colormap", { 0.0, 0.0, 0.0, 0.0, 0.4, 0.9, 0.0, 0.0, 0.8, 0.9, 0.9, 0.0, 1.0, 1.0, 1.0, 1.0 });
  this->set("denoise", false);
  this->set("depth-peeling", false);
  this->set("edges", false);
  this->set("filename", false);
  this->set("fps", false);
  this->set("fxaa", false);
  this->set("grid", false);
  this->set("inverse", false);
  this->set("metadata", false);
  this->set("point-sprites", false);
  this->set("range", std::vector<double>());
  this->set("raytracing", false);
  this->set("samples", 5);
  this->set("ssao", false);
  this->set("tone-mapping", false);
  this->set("trackball", false);
  this->set("volume", false);

  // After initialization, set CheckExists flag to true
  this->Internals->CheckExists = true;
};

//----------------------------------------------------------------------------
options::~options() = default;

//----------------------------------------------------------------------------
options::options(const options& opt)
{
  this->Internals->Options = opt.Internals->Options;
}

//----------------------------------------------------------------------------
options& options::operator=(const options& opt)
{
  this->Internals->Options = opt.Internals->Options;
  return *this;
}

//----------------------------------------------------------------------------
void options::set(const std::string& name, bool value)
{
  this->Internals->set(name, value);
}

//----------------------------------------------------------------------------
void options::set(const std::string& name, int value)
{
  this->Internals->set(name, value);
}

//----------------------------------------------------------------------------
void options::set(const std::string& name, double value)
{
  this->Internals->set(name, value);
}

//----------------------------------------------------------------------------
void options::set(const std::string& name, const std::string& value)
{
  this->Internals->set(name, value);
}

//----------------------------------------------------------------------------
void options::set(const std::string& name, const char* value)
{
  this->Internals->set(name, std::string(value));
}

//----------------------------------------------------------------------------
void options::set(const std::string& name, const std::vector<int>& values)
{
  this->Internals->set(name, values);
}

//----------------------------------------------------------------------------
void options::set(const std::string& name, const std::vector<double>& values)
{
  this->Internals->set(name, values);
}

//----------------------------------------------------------------------------
void options::set(const std::string& name, std::initializer_list<int> values)
{
  this->Internals->set(name, std::vector<int>(values));
}

//----------------------------------------------------------------------------
void options::set(const std::string& name, std::initializer_list<double> values)
{
  this->Internals->set(name, std::vector<double>(values));
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

}

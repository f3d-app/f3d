#include "f3d_options.h"

#include "F3DConfig.h"
#include "F3DOperators.h"

#include <map>

namespace f3d
{
//----------------------------------------------------------------------------
class options::F3DInternals
{
public:
  std::map<std::string, std::string> Options;
};

//----------------------------------------------------------------------------
options::options()
  : Internals(new options::F3DInternals){};

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
template<typename T>
void options::set(const std::string& name, const T& value)
{
  std::stringstream ss;
  ss << value;
  this->Internals->Options[name] = ss.str();
}

//----------------------------------------------------------------------------
template void options::set<>(const std::string& name, const bool& value);
template void options::set<>(const std::string& name, const int& value);
template void options::set<>(const std::string& name, const double& value);
template void options::set<>(const std::string& name, const std::string& value);
template void options::set<>(const std::string& name, const std::vector<int>& value);
template void options::set<>(const std::string& name, const std::vector<double>& value);

//----------------------------------------------------------------------------
template<typename T>
void options::get(const std::string& name, T& value) const
{
  std::stringstream ss(this->Internals->Options.at(name));
  ss >> value;
}

//----------------------------------------------------------------------------
template void options::get<>(const std::string& name, bool& value) const;
template void options::get<>(const std::string& name, int& value) const;
template void options::get<>(const std::string& name, double& value) const;
template void options::get<>(const std::string& name, std::string& value) const;
template void options::get<>(const std::string& name, std::vector<int>& value) const;
template void options::get<>(const std::string& name, std::vector<double>& value) const;

//----------------------------------------------------------------------------
template<typename T>
T options::get(const std::string& name) const
{
  T value;
  std::stringstream ss(this->Internals->Options.at(name));
  ss >> value;
  return value;
}

//----------------------------------------------------------------------------
template bool options::get<>(const std::string& name) const;
template int options::get<>(const std::string& name) const;
template double options::get<>(const std::string& name) const;
template std::string options::get<>(const std::string& name) const;
template std::vector<int> options::get<>(const std::string& name) const;
template std::vector<double> options::get<>(const std::string& name) const;

//----------------------------------------------------------------------------
void options::initialize(options& options)
{
  // General
  options.set("quiet", false);
  options.set("verbose", false);

  // Loader/Loading
  options.set("animation-index", 0);
  options.set("geometry-only", false);
  options.set("progress", false);
  options.set("camera-index", 0);
  options.set("color", std::vector<double>({ 1., 1., 1. }));
  options.set("emissive-factor", std::vector<double>({ 1., 1., 1. }));
  options.set("line-width", 1.0);
  options.set("metallic", 0.0);
  options.set("normal-scale", 1.0);
  options.set("opacity", 1.0);
  options.set("point-size", 10.0);
  options.set("roughness", 0.3);
  options.set("texture-base-color", "");
  options.set("texture-emissive", "");
  options.set("texture-material", "");
  options.set("texture-normal", "");

  // Loading but should not
  options.set("cells", false);
  options.set("scalars", F3DReservedString);
  options.set("component", -1);
  options.set("fullscreen", false);
  options.set("resolution", std::vector<int>(1000, 600));
  options.set("hdri", "");
  options.set("background-color", std::vector<double>({ 0.2, 0.2, 0.2 }));
  options.set("up", "+Y");
  options.set("font-file", "");

  // Rendering/Dynamic
  options.set("axis", false);
  options.set("bar", false);
  options.set("blur-background", false);
  options.set("camera-azimuth-angle", 0.0);
  options.set("camera-elevation-angle", 0.0);
  options.set("camera-focal-point", std::vector<double>());
  options.set("camera-position", std::vector<double>());
  options.set("camera-view-angle", 0.0);
  options.set("camera-view-up", std::vector<double>());
  options.set("colormap",
    std::vector<double>(
      { 0.0, 0.0, 0.0, 0.0, 0.4, 0.9, 0.0, 0.0, 0.8, 0.9, 0.9, 0.0, 1.0, 1.0, 1.0, 1.0 }));
  options.set("denoise", false);
  options.set("depth-peeling", false);
  options.set("edges", false);
  options.set("filename", false);
  options.set("fps", false);
  options.set("fxaa", false);
  options.set("grid", false);
  options.set("inverse", false);
  options.set("metadata", false);
  options.set("point-sprites", false);
  options.set("range", std::vector<double>());
  options.set("raytracing", false);
  options.set("samples", 5);
  options.set("ssao", false);
  options.set("tone-mapping", false);
  options.set("trackball", false);
  options.set("volume", false);
}
}

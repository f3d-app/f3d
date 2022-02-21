#include "f3d_options.h"

#include "F3DConfig.h"
#include "F3DOperators.h"

#include "F3DLog.h"

#include <map>

namespace f3d
{
//----------------------------------------------------------------------------
class options::F3DInternals
{
public:
  std::map<std::string, std::string> Options;
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
  this->set("color", std::vector<double>({ 1., 1., 1. }));
  this->set("emissive-factor", std::vector<double>({ 1., 1., 1. }));
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
  this->set("resolution", std::vector<int>(1000, 600));
  this->set("hdri", "");
  this->set("background-color", std::vector<double>({ 0.2, 0.2, 0.2 }));
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
  this->set("colormap",
    std::vector<double>(
      { 0.0, 0.0, 0.0, 0.0, 0.4, 0.9, 0.0, 0.0, 0.8, 0.9, 0.9, 0.0, 1.0, 1.0, 1.0, 1.0 }));
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
template<typename T>
void options::set(const std::string& name, const T& value)
{
  if (this->Internals->CheckExists &&
    this->Internals->Options.find(name) == this->Internals->Options.end())
  {
    F3DLog::Print(F3DLog::Severity::Error, "Options ", name, " does not exist");
    return;
  }

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
  if (this->Internals->CheckExists &&
    this->Internals->Options.find(name) == this->Internals->Options.end())
  {
    F3DLog::Print(F3DLog::Severity::Error, "Options ", name, " does not exist");
    return;
  }

  std::stringstream ss(this->Internals->Options[name]);
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
}

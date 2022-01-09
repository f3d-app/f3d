#include "f3d_options.h"

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
options::options() : Internals(new options::F3DInternals) {};
options::~options() = default;

//----------------------------------------------------------------------------
template <typename T>
void options::set(const std::string &name, const T& value)
{
  std::stringstream ss;
  ss << value;
  this->Internals->Options[name] = ss.str();
}

//----------------------------------------------------------------------------
template void options::set<bool>(const std::string &name, const bool& value);
template void options::set<int>(const std::string &name, const int& value);
template void options::set<double>(const std::string &name, const double& value);
template void options::set<std::string>(const std::string &name, const std::string& value);
template void options::set<std::vector<int>>(const std::string &name, const std::vector<int>& value);
template void options::set<std::vector<double>>(const std::string &name, const std::vector<double>& value);


//----------------------------------------------------------------------------
template <typename T>
void options::get(const std::string &name, T& value) const
{
  std::stringstream ss(this->Internals->Options.at(name));
  ss >> value;
}

//----------------------------------------------------------------------------
template void options::get<bool>(const std::string &name, bool& value) const;
template void options::get<int>(const std::string &name, int& value) const;
template void options::get<double>(const std::string &name, double& value) const;
template void options::get<std::string>(const std::string &name, std::string& value) const;
template void options::get<std::vector<int>>(const std::string &name, std::vector<int>& value) const;
template void options::get<std::vector<double>>(const std::string &name, std::vector<double>& value) const;

//----------------------------------------------------------------------------
template <typename T>
T options::get(const std::string &name) const
{
  T value;
  std::stringstream ss(this->Internals->Options.at(name));
  ss >> value;
  return value;
}

//----------------------------------------------------------------------------
template bool options::get<bool>(const std::string &name) const;
template int options::get<int>(const std::string &name) const;
template double options::get<double>(const std::string &name) const;
template std::string options::get<std::string>(const std::string &name) const;
template std::vector<int> options::get<std::vector<int>>(const std::string &name) const;
template std::vector<double> options::get<std::vector<double>>(const std::string &name) const;
}

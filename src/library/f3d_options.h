#ifndef f3d_options_h
#define f3d_options_h

#include "f3d_export.h"

#include <string>
#include <vector>

// TODO documentation

namespace f3d
{
class F3D_EXPORT options
{
public:
  options();
  ~options();

  void set(const std::string& name, bool value);
  void set(const std::string& name, int value);
  void set(const std::string& name, double value);
  void set(const std::string& name, const std::string& value);
  void set(const std::string& name, const char* value);
  void set(const std::string& name, const std::vector<int>& values);
  void set(const std::string& name, const std::vector<double>& values);
  void set(const std::string& name, std::initializer_list<int> values);
  void set(const std::string& name, std::initializer_list<double> values);

  void get(const std::string& name, bool& value) const;
  void get(const std::string& name, int& value) const;
  void get(const std::string& name, double& value) const;
  void get(const std::string& name, std::string& value) const;
  void get(const std::string& name, std::vector<int>& value) const;
  void get(const std::string& name, std::vector<double>& value) const;

  bool getAsBool(const std::string& name) const;
  int getAsInt(const std::string& name) const;
  double getAsDouble(const std::string& name) const;
  std::string getAsString(const std::string& name) const;
  std::vector<int> getAsIntVector(const std::string& name) const;
  std::vector<double> getAsDoubleVector(const std::string& name) const;
  
  bool& getAsBoolRef(const std::string& name) const;

private:
  class F3DInternals;
  F3DInternals* Internals;
  options(const options& opt) = delete;
  options& operator=(const options& opt) = delete;
};
}

#endif

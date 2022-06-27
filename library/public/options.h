/**
 * @class   options
 * @brief   Class used to control the different options
 *
 * A class to control the different options of f3d.
 * See the TODO for the full listing of options
 */

#ifndef f3d_options_h
#define f3d_options_h

#include "export.h"

#include <stdexcept>
#include <string>
#include <vector>

namespace f3d
{
class F3D_EXPORT options
{
public:
  //@{
  /**
   * Default/Copy/move constructors/operators
   */
  options();
  ~options();
  options(const options& opt);
  options& operator=(const options& opt);
  options(options&& other);
  options& operator=(options&& other);
  //@}

  class exception : public std::runtime_error
  {
  public:
    exception(const std::string& what = "")
      : std::runtime_error(what)
    {
    }
  };

  //@{
  /**
   * Setters for all supported types
   */
  options& set(const std::string& name, bool value);
  options& set(const std::string& name, int value);
  options& set(const std::string& name, double value);
  options& set(const std::string& name, const std::string& value);
  options& set(const std::string& name, const char* value);
  options& set(const std::string& name, const std::vector<int>& values);
  options& set(const std::string& name, const std::vector<double>& values);
  options& set(const std::string& name, std::initializer_list<int> values);
  options& set(const std::string& name, std::initializer_list<double> values);
  //@}

  //@{
  /**
   * Reference getters for all supported types
   */
  void get(const std::string& name, bool& value) const;
  void get(const std::string& name, int& value) const;
  void get(const std::string& name, double& value) const;
  void get(const std::string& name, std::string& value) const;
  void get(const std::string& name, std::vector<int>& value) const;
  void get(const std::string& name, std::vector<double>& value) const;
  //@}

  //@{
  /**
   * Explicit getters for all supported types
   */
  bool getAsBool(const std::string& name) const;
  int getAsInt(const std::string& name) const;
  double getAsDouble(const std::string& name) const;
  std::string getAsString(const std::string& name) const;
  std::vector<int> getAsIntVector(const std::string& name) const;
  std::vector<double> getAsDoubleVector(const std::string& name) const;
  //@}

  //@{
  /**
   * Explicit reference getters for all supported types.
   * Can throw a options::exception in case of failure.
   */
  bool& getAsBoolRef(const std::string& name);
  int& getAsIntRef(const std::string& name);
  double& getAsDoubleRef(const std::string& name);
  std::string& getAsStringRef(const std::string& name);
  std::vector<int>& getAsIntVectorRef(const std::string& name);
  std::vector<double>& getAsDoubleVectorRef(const std::string& name);
  //@}

  /**
   * A boolean option specific method to toggle it
   */
  options& toggle(const std::string& name);

private:
  class internals;
  internals* Internals;
};
}

#endif

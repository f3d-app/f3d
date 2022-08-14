/**
 * @class   options
 * @brief   Class used to control the different options
 *
 * A class to control the different options of f3d.
 * See the README_libf3d.md for the full listing of options
 */

#ifndef f3d_options_h
#define f3d_options_h

#include "exception.h"
#include "export.h"

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
  options& operator=(const options& opt) noexcept;
  options(options&& other) noexcept;
  options& operator=(options&& other) noexcept;
  //@}

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
   * Copy the option value into the provided reference, for all supported types,
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
   * Explicit getters to actual reference to the options variable, for all supported types.
   * Modifying the returned reference will modify the option.
   * Throw an options::incompatible_exception if the type is not compatible with the option.
   * Throw an options::inexistent_exception if option does not exist.
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

  //@{
  /**
   * Options specific exceptions
   */
  struct incompatible_exception : public exception { incompatible_exception(const std::string& what = ""); };
  struct inexistent_exception : public exception { inexistent_exception(const std::string& what = ""); };
  //@}

private:
  class internals;
  internals* Internals;
};
}

#endif

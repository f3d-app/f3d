#ifndef f3d_options_h
#define f3d_options_h

#include "exception.h"
#include "export.h"

#include <string>
#include <vector>
#include <variant>
#include <array>

namespace f3d
{

typedef std::variant<
  double,
  int
> variant_t;

struct f3d_options {
  struct scene_t {
    struct animation_t {
      int index = 0;
    };
  animation_t animation;
  };
 scene_t scene;
 struct render_t {
   double line_width = 1.0;
 };
 render_t render;
};

/**
 * @class   options
 * @brief   Class used to control the different options
 *
 * A class to control the different options of f3d.
 * See the README_libf3d.md for the full listing of options
 */
class F3D_EXPORT options
{
public:
  ///@{ @name Constructors
  /**
   * Default/Copy/move constructors/operators.
   */
  options();
  ~options();
  options(const options& opt);
  options& operator=(const options& opt) noexcept;
  options(options&& other) noexcept;
  options& operator=(options&& other) noexcept;
  ///@}

  void setVariant(const std::string& name, variant_t value);
  variant_t getVariant(const std::string& name);
  void setString(const std::string& name, std::string value);
  std::string getString(const std::string& name);


  ///@{ @name Setters
  /**
   * Setters for all supported types.
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
  ///@}

  ///@{ @name Reference Getters
  /**
   * Copy the option value into the provided reference, for all supported types.
   */
  void get(const std::string& name, bool& value) const;
  void get(const std::string& name, int& value) const;
  void get(const std::string& name, double& value) const;
  void get(const std::string& name, std::string& value) const;
  void get(const std::string& name, std::vector<int>& value) const;
  void get(const std::string& name, std::vector<double>& value) const;
  ///@}

  ///@{ @name Explicit Copy Getters
  /**
   * Explicit getters for all supported types.
   */
  bool getAsBool(const std::string& name) const;
  int getAsInt(const std::string& name) const;
  double getAsDouble(const std::string& name) const;
  std::string getAsString(const std::string& name) const;
  std::vector<int> getAsIntVector(const std::string& name) const;
  std::vector<double> getAsDoubleVector(const std::string& name) const;
  ///@}

  ///@{ @name Explicit Reference Getters
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
  ///@}

  /**
   * A boolean option specific method to toggle it.
   */
  options& toggle(const std::string& name);

  /**
   * Compare an option between this and a provided other.
   * Return true is they are the same value, false otherwise.
   * Throw an options::inexistent_exception if option does not exist.
   */
  bool isSame(const options& other, const std::string& name) const;

  /**
   * Copy the value of an option from this to the provided other.
   * Throw an options::inexistent_exception if option does not exist.
   */
  options& copy(const options& other, const std::string& name);

  /**
   * Get all available option names.
   */
  std::vector<std::string> getNames();

  /**
   * Get the closest option name and its Levenshtein distance.
   */
  std::pair<std::string, unsigned int> getClosestOption(const std::string& option) const;

  /**
   * An exception that can be thrown by the options
   * when a provided option type is incompatible with
   * its internal type.
   */
  struct incompatible_exception : public exception
  {
    explicit incompatible_exception(const std::string& what = "");
  };

  /**
   * An exception that can be thrown by the options
   * when a provided option does not exist.
   */
  struct inexistent_exception : public exception
  {
    explicit inexistent_exception(const std::string& what = "");
  };

  f3d_options& getStruct();
  const f3d_options& getConstStruct() const;

private:
  class internals;
  internals* Internals;
};
}

#endif

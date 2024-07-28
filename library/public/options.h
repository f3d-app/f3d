#ifndef f3d_options_h
#define f3d_options_h

#include "options_struct.h"
#include "exception.h"
#include "export.h"
#include "types.h"

#include <string>
#include <vector>
#include <variant>
#include <array>

namespace f3d
{
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

  /**
   * Set/Get an option as a variant based on its name
   * Throw an options::inexistent_exception if option does not exist.
   * Throw an options::incompatible_exception if value is not compatible with option.
   * TODO Improve type compatibilty, eg avoid doing f3d_ratio_t(2.5) ?
   */
  options& set(const std::string& name, option_variant_t value);
  option_variant_t get(const std::string& name);

  /**
   * Set/Get an option as a string based on its name
   * The setter use specific parsing, see the related doc TODO
   * Throw an options::inexistent_exception if option does not exist.
   */
  options& setAsString(const std::string& name, std::string str);
  std::string getAsString(const std::string& name);

  /**
   * A boolean option specific method to toggle it.
   * Throw an options::inexistent_exception if option does not exist.
   * Throw an options::incompatible_exception if option is not boolean.
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
   * TODO: Add a getNamesStruct ?
   */
  std::vector<std::string> getNames();

  /**
   * Get the closest option name and its Levenshtein distance.
   */
  std::pair<std::string, unsigned int> getClosestOption(const std::string& option) const;

  /**
   * An exception that can be thrown by the options
   * when parsing of a string into an option value fails
   */
  struct parsing_exception : public exception
  {
    explicit parsing_exception(const std::string& what = "");
  };

  /**
   * An exception that can be thrown by the options
   * when an operation on a specific option is incompatible with
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

  options_struct& getStruct();
  const options_struct& getStruct() const;

private:
  class internals;
  internals* Internals;
};
}

#endif

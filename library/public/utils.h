#ifndef f3d_utils_h
#define f3d_utils_h

#include "exception.h"
#include "export.h"

#include <filesystem>
#include <map>
#include <regex>
#include <sstream>
#include <string>
#include <vector>

namespace f3d
{
/**
 * @class   utils
 * @brief   Class containing useful static functions
 *
 * A class containing useful static functions unrelated to other classes.
 *
 */
class F3D_EXPORT utils
{
public:
  /**
   * Compute the Levenshtein distance between two strings.
   * Can be useful for spell checking and typo detection.
   */
  [[nodiscard]] static unsigned int textDistance(std::string_view strA, std::string_view strB);

  // clang-format off
  /**
   * Tokenize provided string_view into a vector of strings, using the same logic as bash.
   *  - Split by spaces unless between quotes
   *  - Split by quoted section and remove the quotes
   *  - Supported quotes are: '"`
   *  - Use escaped \ quotes, spaces and escape to add them verbatim
   *  - Comments are supported with `#`, any characters after are ignored
   *  - Use escaped \# to add it verbatim
   *  - Other escaped characters are also added verbatim
   * Throw a tokenize_exception if a quoted section is not closed or if finishing with an escape
   *
   * Examples:
   * `set scene.up.direction +Z` -> `set` `scene.up.direction` `+Z`
   * `set render.hdri.file "/path/to/file with spaces.png"` -> `set`, `render.hdri.file`, `/path/to/file with spaces.png`
   * `set render.hdri.file '/path/to/file with spaces.png'` -> `set`, `render.hdri.file`, `/path/to/file with spaces.png`
   * `set render.hdri.file "/path/to/file'with'quotes.png"` -> `set`, `render.hdri.file`, `/path/to/file'with'quotes.png`
   * `set render.hdri.file /path/to/file\ spaces\ \'quotes\".png` -> `set`, `render.hdri.file`, `/path/to/file spaces 'quotes".png`
   * `set render.hdri.file C:\\path\\to\\windows\\file.png` -> `set`, `render.hdri.file`, `C:\path\to\windows\file.png`
   * `set scene.up.direction +Z # A comment` -> `set`, `scene.up.direction`, `+Z`
   * `set scene.up.direction +\Z` -> `set`, `scene.up.direction`, `+Z`
   * `set scene.up.direction "+Z` -> tokenize_exception
   * `set scene.up.direction +Z\` -> tokenize_exception
   */
  [[nodiscard]] static std::vector<std::string> tokenize(std::string_view str);
  // clang-format on

  /**
   * Collapse a string filesystem path by:
   * - Expanding tilda `~` into home dir in a cross-platform way
   * - Transform relative path into an absolute path based on basedDirectory if provided, or the
   * current directory if not
   * - Remove any `..` if any
   * Rely on vtksys::SystemTools::CollapseFullPath but return empty string if the provided
   * string is empty.
   */
  [[nodiscard]] static std::filesystem::path collapsePath(
    const std::filesystem::path& path, const std::filesystem::path& baseDirectory = {});

  /**
   * An exception that can be thrown by tokenize
   */
  struct tokenize_exception : public exception
  {
    explicit tokenize_exception(const std::string& what = "");
  };

  /** String template allowing substitution of variables enclosed in curly braces.
    ```
    string_template("{greeting} {name}!")
      .substitute({ { "greeting", "hello" }, { "name", "World" } })
      .str() == "hello World!"
    ```
   */
  class string_template
  {
    std::vector<std::pair<std::string, bool>> fragments;

  public:
    explicit string_template(const std::string& templateString);

    /** Substitute variables based on a `std::string(const std::string&)` function.
     * Variables for which the function throws a `string_template::lookup_error` exception
     * are left untouched.
     */
    template<typename F>
    string_template& substitute(F lookup);

    /** Substitute variables based on a map.
     * Variables for which the map does not contain a key are left untouched.
     */
    string_template& substitute(const std::map<std::string, std::string>& lookup);

    /** Return a string representation of the string template */
    [[nodiscard]] std::string str() const;

    /** List the remaining un-substituted variables. */
    [[nodiscard]] std::vector<std::string> variables() const;

    /**
     * Exception to be thrown by substitution functions to let untouched variables through.
     */
    struct lookup_error : public std::out_of_range
    {
      explicit lookup_error(const std::string& what = "")
        : std::out_of_range(what)
      {
      }
    };
  };
};

//------------------------------------------------------------------------------
inline utils::string_template::string_template(const std::string& templateString)
{
  const std::string varName = "[\\w_.%:-]+";
  const std::string escapedVar = "(\\{(\\{" + varName + "\\})\\})";
  const std::string substVar = "(\\{(" + varName + ")\\})";
  const std::regex escapedVarRe(escapedVar);
  const std::regex substVarRe(substVar);

  const auto callback = [&](const std::string& m)
  {
    if (std::regex_match(m, escapedVarRe))
    {
      this->fragments.emplace_back(std::regex_replace(m, escapedVarRe, "$2"), false);
    }
    else if (std::regex_match(m, substVarRe))
    {
      this->fragments.emplace_back(std::regex_replace(m, substVarRe, "$2"), true);
    }
    else
    {
      this->fragments.emplace_back(m, false);
    }
  };

  const std::regex re(escapedVar + "|" + substVar);
  std::sregex_token_iterator begin(templateString.begin(), templateString.end(), re, { -1, 0 });
  std::for_each(begin, std::sregex_token_iterator(), callback);
}

//------------------------------------------------------------------------------
template<typename F>
utils::string_template& utils::string_template::substitute(F lookup)
{
  for (auto& [fragment, isVariable] : this->fragments)
  {
    if (isVariable)
    {
      try
      {
        fragment = lookup(fragment);
        isVariable = false;
      }
      catch (const lookup_error&)
      {
        /* leave variable as is */
      }
    }
  }
  return *this;
}

//------------------------------------------------------------------------------
inline utils::string_template& utils::string_template::substitute(
  const std::map<std::string, std::string>& lookup)
{
  return this->substitute(
    [&](const std::string& key)
    {
      try
      {
        return lookup.at(key);
      }
      catch (const std::out_of_range&)
      {
        throw lookup_error(key);
      }
    });
}

//------------------------------------------------------------------------------
inline std::string utils::string_template::str() const
{
  std::ostringstream ss;
  // cppcheck-suppress unassignedVariable
  // (false positive, fixed in cppcheck 2.8)
  for (const auto& [fragment, isVariable] : this->fragments)
  {
    if (isVariable)
    {
      ss << "{" << fragment << "}";
    }
    else
    {
      ss << fragment;
    }
  }
  return ss.str();
}

//------------------------------------------------------------------------------
inline std::vector<std::string> utils::string_template::variables() const
{
  std::vector<std::string> variables;
  for (const auto& [fragment, isVariable] : this->fragments)
  {
    if (isVariable)
    {
      variables.emplace_back(fragment);
    }
  }
  return variables;
}
}

#endif

#ifndef f3d_utils_h
#define f3d_utils_h

#include "exception.h"
#include "export.h"

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
  static unsigned int textDistance(const std::string& strA, const std::string& strB);

  // clang-format off
  /**
   * Tokenize provided string_view into a vector of strings, using the same logic as bash.
   *  - Split by spaces unless between quotes
   *  - Split by quoted section and remove the quotes
   *  - Supported quotes are: '"`
   *  - Use escaped \ quotes, spaces and escape to add them verbatim
   *  - Other escaped characters are also added verbatim
   * Throw a tokenize_exception if a quoted section is not closed or if finishing with an escape
   *
   * Examples:
   * `set scene.up.direction +Z` -> `set` `scene.up.direction` `+Z`
   * `set render.hdri.file "/path/to/file with spaces.png"` -> `set`, `render.hdri.file`,
   * `/path/to/file with spaces.png` `set render.hdri.file '/path/to/file with spaces.png'` ->
   * `set`, `render.hdri.file`, `/path/to/file with spaces.png` `set render.hdri.file
   * "/path/to/file'with'quotes.png"` -> `set`, `render.hdri.file`, `/path/to/file'with'quotes.png`
   * `set render.hdri.file /path/to/file\ spaces\ \'quotes\".png` -> `set`, `render.hdri.file`,
   * `/path/to/file spaces 'quotes".png` `set render.hdri.file C:\\path\\to\\windows\\file.png` ->
   * `set`, `render.hdri.file`, `C:\path\to\windows\file.png` `set scene.up.direction +\Z` -> `set`,
   * `scene.up.direction`, `+Z` `set scene.up.direction "+Z` -> tokenize_exception `set
   * scene.up.direction +Z\` -> tokenize_exception
   */
  static std::vector<std::string> tokenize(std::string_view str);
  // clang-format on

  /**
   * An exception that can be thrown by tokenize
   */
  struct tokenize_exception : public exception
  {
    explicit tokenize_exception(const std::string& what = "");
  };
};
}

#endif

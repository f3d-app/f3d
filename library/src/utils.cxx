#include "utils.h"

#include "levenshtein.h"
#include "log.h"

#include <vtksys/Encoding.hxx>
#include <vtksys/SystemTools.hxx>

#if defined(_WIN32)
// clang-format off
#include <Windows.h>
#include <Knownfolders.h>
#include <shlobj_core.h>
// clang-format on
#endif

namespace fs = std::filesystem;

namespace f3d
{
//----------------------------------------------------------------------------
unsigned int utils::textDistance(std::string_view strA, std::string_view strB)
{
  return static_cast<unsigned int>(detail::levenshtein(strA, strB));
}

//----------------------------------------------------------------------------
std::vector<std::string> utils::tokenize(std::string_view str, bool keepComments)
{
  std::vector<std::string> tokens;
  std::string token;
  const auto accumulate = [&](const char& c) { token.push_back(c); };
  const auto emit = [&]()
  {
    if (!token.empty())
    {
      tokens.push_back(token);
      token.clear();
    }
  };
  bool escaped = false;
  char quoted = '\0';
  bool commented = false;
  for (char c : str)
  {
    switch (c)
    {
      case '\\':
        if (escaped)
        {
          accumulate(c);
        }
        escaped = !escaped;
        break;
      case ' ':
        if (!escaped && !quoted)
        {
          emit();
        }
        else
        {
          accumulate(c);
        }
        escaped = false;
        break;
      case '"':
      case '\'':
      case '`':
        if (!escaped && quoted == c)
        {
          emit();
          quoted = '\0';
        }
        else if (!escaped && !quoted)
        {
          quoted = c;
        }
        else
        {
          accumulate(c);
        }
        escaped = false;
        break;
      case '#':
        if (!escaped && !quoted && keepComments)
        {
          commented = true;
        }
        else
        {
          accumulate(c);
        }
        escaped = false;
        break;
      default:
        accumulate(c);
        escaped = false;
        break;
    }

    if (commented)
    {
      break;
    }
  }
  if (quoted || escaped)
  {
    throw tokenize_exception();
  }
  emit();
  return tokens;
}

//----------------------------------------------------------------------------
fs::path utils::collapsePath(const fs::path& path, const fs::path& baseDirectory)
{
  try
  {
    return path.empty() ? path
      : baseDirectory.empty()
      ? fs::path(vtksys::SystemTools::CollapseFullPath(path.string()))
      : fs::path(vtksys::SystemTools::CollapseFullPath(path.string(), baseDirectory.string()));
  }
  catch (const fs::filesystem_error& ex)
  {
    log::error("Could not collapse path: ", ex.what());
    return {};
  }
}

//----------------------------------------------------------------------------
std::string utils::globToRegex(std::string_view glob, char pathSeparator)
{
  bool supportGlobStars = [glob]()
  {
    // find if the glob expression contains a globstar (**)
    if (glob.size() < 2)
    {
      return false;
    }

    bool escaped = false;
    int starCount = 0;
    for (char c : glob)
    {
      if (c == '\\')
      {
        escaped = !escaped;
        starCount = 0;
        continue;
      }

      starCount = !escaped && c == '*' ? starCount + 1 : 0;

      if (starCount >= 2)
      {
        return true;
      }

      escaped = false;
    }

    return false;
  }();
  const std::string globSeparator = pathSeparator == '\\' ? "\\\\" : "/";

  std::string result;

  bool escaped = false;
  bool inCharClass = false;
  std::vector<size_t> alternations;

  for (size_t i = 0; i < glob.size(); i++)
  {
    char c = glob[i];

    switch (c)
    {
      case '*':
        if (escaped || inCharClass)
        {
          result += c;
        }
        else
        {
          bool prevTokenSepOrBeg = i == 0 ||
            (i >= globSeparator.size()
                ? glob.substr(i - globSeparator.size(), globSeparator.size()) == globSeparator
                : false);
          unsigned starCount = 1;
          while (i + 1 < glob.size() && glob[i + 1] == '*')
          {
            starCount++;
            i++;
          }
          bool nextTokenSepOrEnd =
            i + 1 >= glob.size() || glob.substr(i + 1, globSeparator.size()) == globSeparator;
          if (supportGlobStars)
          {
            if (starCount > 1 && prevTokenSepOrBeg && nextTokenSepOrEnd)
            {
              result += "(?:[^";
              result += globSeparator;
              result += "]*(?:";
              result += globSeparator;
              result += "|$))*";
              i += globSeparator.size(); // Eat separator if next
            }
            else
            {
              result += "[^";
              result += globSeparator;
              result += "]*";
            }
          }
          else
          {
            result += ".*";
          }
        }
        escaped = false;
        break;
      case '?':
        if (escaped || inCharClass)
        {
          result += '?';
        }
        else if (supportGlobStars)
        {
          result += "[^";
          result += globSeparator;
          result += "]";
        }
        else
        {
          result += '.';
        }
        escaped = false;
        break;
      case '[':
        result += '[';
        if (!escaped && !inCharClass)
        {
          inCharClass = true;
          if (i + 1 < glob.size() && (glob[i + 1] == '!' || glob[i + 1] == '^'))
          {
            result += '^';
            i++; // Eat the '!' or '^'
          }
        }
        escaped = false;
        break;
      case ']':
        result += ']';
        if (!escaped)
        {
          inCharClass = false;
        }
        escaped = false;
        break;
      case '{':
        if (!escaped && !inCharClass)
        {
          alternations.push_back(i);
          result += "(?:";
        }
        else
        {
          result += '{';
        }
        escaped = false;
        break;
      case '}':
        if (!escaped && !inCharClass && !alternations.empty())
        {
          result += ')';
          alternations.pop_back();
        }
        else
        {
          result += '}';
        }
        escaped = false;
        break;
      case ',':
        if (!escaped && !inCharClass && !alternations.empty())
        {
          result += '|';
        }
        else
        {
          result += ',';
        }
        escaped = false;
        break;
      case '\\':
        result += c;
        escaped = !escaped;
        break;
      case '.':
      case '(':
      case ')':
      case '+':
      case '|':
      case '^':
      case '$':
        if (!inCharClass)
        {
          result += '\\';
        }
        result += c;
        escaped = false;
        break;
      default:
        result += c;
        escaped = false;
        break;
    }
  }

  if (escaped)
  {
    throw glob_exception("Escape at end of glob expression");
  }
  if (inCharClass || !alternations.empty())
  {
    throw glob_exception(
      "Unmatched '" + std::string(inCharClass ? "]" : "}") + "' in glob expression");
  }

  return result;
}

//----------------------------------------------------------------------------
std::optional<std::string> utils::getEnv(const std::string& env)
{
  std::string val;
  if (vtksys::SystemTools::GetEnv(env, val))
  {
    return val;
  }
  return std::nullopt;
}

//----------------------------------------------------------------------------
std::optional<std::string> utils::getKnownFolder(KnownFolder knownFolder)
{
#if defined(_WIN32)
  KNOWNFOLDERID winKnowFolder;
  switch (knownFolder)
  {
    case KnownFolder::ROAMINGAPPDATA:
      winKnowFolder = FOLDERID_RoamingAppData;
      break;
    case KnownFolder::LOCALAPPDATA:
      winKnowFolder = FOLDERID_LocalAppData;
      break;
    case KnownFolder::PICTURES:
      winKnowFolder = FOLDERID_Pictures;
      break;
    default:
      // Unreachable
      return std::nullopt;
      break;
  }

  LPWSTR lpwstr;
  if (FAILED(SHGetKnownFolderPath(winKnowFolder, 0, NULL, &lpwstr)))
  {
    return std::nullopt;
  }

  std::wstring wstr = std::wstring(lpwstr);
  CoTaskMemFree(lpwstr);
  std::string folderVal = vtksys::Encoding::ToNarrow(wstr);
  vtksys::SystemTools::ConvertToUnixSlashes(folderVal);
  return folderVal;
#else
  return std::nullopt;
#endif
}

//----------------------------------------------------------------------------
utils::tokenize_exception::tokenize_exception(const std::string& what)
  : exception(what)
{
}

//----------------------------------------------------------------------------
utils::glob_exception::glob_exception(const std::string& what)
  : exception(what)
{
}
}

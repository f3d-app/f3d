#include "utils.h"

#include "levenshtein.h"

namespace f3d
{
//----------------------------------------------------------------------------
unsigned int utils::textDistance(std::string_view strA, std::string_view strB)
{
  return static_cast<unsigned int>(detail::levenshtein(strA, strB));
}

//----------------------------------------------------------------------------
std::vector<std::string> utils::tokenize(std::string_view str)
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
        if (!escaped && !quoted)
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
utils::tokenize_exception::tokenize_exception(const std::string& what)
  : exception(what)
{
}
}

#include <regex>
#include <sstream>

#include "utils.h"

#include "levenshtein.h"

namespace f3d
{
//----------------------------------------------------------------------------
unsigned int utils::textDistance(const std::string& strA, const std::string& strB)
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
      default:
        accumulate(c);
        escaped = false;
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

//----------------------------------------------------------------------------
utils::StringTemplate::StringTemplate(const std::string& templateString)
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

utils::StringTemplate& utils::StringTemplate::substitute(
  const std::function<std::string(const std::string&)>& lookup)
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
      catch (const std::out_of_range& e)
      {
        /* leave variable as is */
      }
    }
  }
  return *this;
}

utils::StringTemplate& utils::StringTemplate::substitute(
  const std::map<std::string, std::string>& lookup)
{
  return this->substitute([&](const std::string& key) { return lookup.at(key); });
}

std::string utils::StringTemplate::str() const
{
  std::ostringstream ss;
  for (auto [fragment, isVariable] : this->fragments)
    if (isVariable)
      ss << "{" << fragment << "}";
    else
      ss << fragment;
  return ss.str();
}

std::vector<std::string> utils::StringTemplate::variables() const
{
  std::vector<std::string> variables;
  for (auto [fragment, isVariable] : this->fragments)
    if (isVariable)
      variables.emplace_back(fragment);
  return variables;
}

}

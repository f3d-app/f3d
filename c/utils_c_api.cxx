#include "utils_c_api.h"
#include "utils.h"
#include <cstring>
#include <filesystem>
#include <string>
#include <vector>

namespace
{
//----------------------------------------------------------------------------
char* f3d_utils_strdup(const std::string& s)
{
  char* r = new char[s.size() + 1];
  std::memcpy(r, s.c_str(), s.size() + 1);
  return r;
}

//----------------------------------------------------------------------------
void f3d_utils_internal_tokens_free(char** tokens, size_t count)
{
  if (!tokens)
  {
    return;
  }
  for (size_t i = 0; i < count; ++i)
  {
    delete[] tokens[i];
  }
  delete[] tokens;
}
} // namespace

//----------------------------------------------------------------------------
unsigned int f3d_utils_text_distance(const char* str_a, const char* str_b)
{
  std::string a = str_a ? str_a : "";
  std::string b = str_b ? str_b : "";
  return f3d::utils::textDistance(a, b);
}

//----------------------------------------------------------------------------
char** f3d_utils_tokenize(const char* str, int keep_comments, size_t* out_count)
{
  if (!str)
  {
    if (out_count)
    {
      *out_count = 0;
    }
    return nullptr;
  }

  std::vector<std::string> vec = f3d::utils::tokenize(str, keep_comments != 0);

  size_t n = vec.size();
  char** out = new char*[n];
  for (size_t i = 0; i < n; ++i)
  {
    out[i] = f3d_utils_strdup(vec[i]);
  }

  if (out_count)
  {
    *out_count = n;
  }

  return out;
}

//----------------------------------------------------------------------------
void f3d_utils_tokens_free(char** tokens, size_t count)
{
  f3d_utils_internal_tokens_free(tokens, count);
}

//----------------------------------------------------------------------------
char* f3d_utils_collapse_path(const char* path, const char* base_directory)
{
  std::filesystem::path p(path);
  std::filesystem::path base = base_directory ? base_directory : "";
  auto collapsed = f3d::utils::collapsePath(p, base);
  return f3d_utils_strdup(collapsed.string());
}

//----------------------------------------------------------------------------
char* f3d_utils_glob_to_regex(const char* glob, char path_separator)
{
  std::string regex = f3d::utils::globToRegex(glob, path_separator);
  return f3d_utils_strdup(regex);
}

//----------------------------------------------------------------------------
char* f3d_utils_get_env(const char* env)
{
  auto opt = f3d::utils::getEnv(std::string(env));
  if (!opt.has_value())
  {
    return nullptr;
  }
  return f3d_utils_strdup(*opt);
}

//----------------------------------------------------------------------------
char* f3d_utils_get_known_folder(f3d_utils_known_folder_t known_folder)
{
  f3d::utils::KnownFolder kf;
  switch (known_folder)
  {
    case F3D_UTILS_KNOWN_FOLDER_ROAMINGAPPDATA:
      kf = f3d::utils::KnownFolder::ROAMINGAPPDATA;
      break;
    case F3D_UTILS_KNOWN_FOLDER_LOCALAPPDATA:
      kf = f3d::utils::KnownFolder::LOCALAPPDATA;
      break;
    case F3D_UTILS_KNOWN_FOLDER_PICTURES:
      kf = f3d::utils::KnownFolder::PICTURES;
      break;
    default:
      return nullptr;
  }

  auto opt = f3d::utils::getKnownFolder(kf);
  if (!opt.has_value())
  {
    return nullptr;
  }
  return f3d_utils_strdup(*opt);
}

//----------------------------------------------------------------------------
void f3d_utils_string_free(char* str)
{
  delete[] str;
}

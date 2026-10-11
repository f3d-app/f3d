#include "F3DUtils.h"

#include <vtkObject.h>
#include <vtkSetGet.h>

#ifdef _WIN32
#include <Windows.h>
#endif

#include <charconv>
#include <stdexcept>

#ifdef F3D_MODULE_CLIP
#include "clip/clip.h"
#endif

//----------------------------------------------------------------------------
double F3DUtils::ParseToDouble(const std::string& str, double def, const std::string& nameError)
{
  double value = def;
  if (!str.empty())
  {
    try
    {
      value = std::stod(str);
    }
    catch (const std::invalid_argument&)
    {
      vtkWarningWithObjectMacro(
        nullptr, "Could not parse " << nameError << ": " << str << ". Ignoring.");
    }
    catch (const std::out_of_range&)
    {
      vtkWarningWithObjectMacro(
        nullptr, "Provided " << nameError << " out of range: " << str << ". Ignoring.");
    }
  }
  return value;
}

//----------------------------------------------------------------------------
int F3DUtils::ParseToInt(const std::string& str, int def, const std::string& nameError)
{
  int value = def;
  if (!str.empty())
  {
    auto result = std::from_chars(str.data(), str.data() + str.size(), value);

    if (result.ec == std::errc::result_out_of_range)
    {
      vtkWarningWithObjectMacro(
        nullptr, "Provided " << nameError << " out of range: " << str << ". Ignoring.");
      return def;
    }
    else if (result.ec != std::errc() || *(result.ptr) != '\0')
    {
      vtkWarningWithObjectMacro(
        nullptr, "Could not parse " << nameError << ": " << str << ". Ignoring.");
      return def;
    }
  }
  return value;
}

//----------------------------------------------------------------------------
bool F3DUtils::CopyToClipboard(const std::string& text)
{
#ifdef F3D_MODULE_CLIP
  clip::set_x11_wait_timeout(50);
  try
  {
    return clip::set_text(text);
  }
  // Cannot cover clip failure
  // LCOV_EXCL_START
  catch (const clip::clip_exception& e)
  {
    vtkWarningWithObjectMacro(nullptr, "Clipboard error: " << e.what());
    return false;
  }
  // LCOV_EXCL_STOP
#else
  return false;
#endif
}

//----------------------------------------------------------------------------
bool F3DUtils::GetFromClipboard(std::string& text)
{
#ifdef F3D_MODULE_CLIP
  clip::set_x11_wait_timeout(50);
  try
  {
    return clip::get_text(text);
  }
  // Cannot cover clip failure
  // LCOV_EXCL_START
  catch (const clip::clip_exception& e)
  {
    vtkWarningWithObjectMacro(nullptr, "Clipboard error: " << e.what());
    return false;
  }
  // LCOV_EXCL_STOP
#else
  text.clear();
  return false;
#endif
}

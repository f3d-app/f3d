/**
 * @class   F3DImguiStyle
 * @brief   Namespace containing method to recover color for styling F3D ImGui UI
 *
 * Provide methods to use when recovering colors for F3D ImGui UI
 */

#ifndef F3DImguiStyle_h
#define F3DImguiStyle_h

#include <imgui.h>

namespace F3DImguiStyle
{
constexpr ImVec4 DecomposeImv4(int val)
{
  return {
    ((val >> 16) & 0xff) / 255.f,
    ((val >> 8) & 0xff) / 255.f,
    (val & 0xff) / 255.f,
    1.f,
  };
}

constexpr ImVec4 GetErrorColor()
{
  // red with the same saturation (95%) and luminance (50%) as F3D yellow
  return DecomposeImv4(0xf94306);
}

constexpr ImVec4 GetWarningColor()
{
  // F3D yellow
  return DecomposeImv4(0xf9b208);
}

constexpr ImVec4 GetHighlightColor()
{
  // F3D blue
  return DecomposeImv4(0x788bff);
}

static constexpr ImVec4 GetCompletionColor()
{
  // green with the same saturation (100%) and luminance (74%) as F3D blue
  return DecomposeImv4(0x7aff7a);
}

constexpr ImVec4 GetTextColor()
{
  // F3D white
  return DecomposeImv4(0xf4f4f4);
}

constexpr ImVec4 GetMidColor()
{
  // grey between F3D black and F3D white
  return DecomposeImv4(0x545454);
}

constexpr ImVec4 GetBackgroundColor()
{
  // F3D black
  return DecomposeImv4(0x141414);
}

constexpr float GetDefaultMargin()
{
  return 5.f;
}
};

#endif

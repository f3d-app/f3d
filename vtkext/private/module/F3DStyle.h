/**
 * @class   F3DStyle
 * @brief   Namespace containing method to recover color for styling F3D UI
 *
 * Provide methods to use when recovering colors for F3D UI
 */

#ifndef F3DStyle_h
#define F3DStyle_h

#include <tuple>

struct ImVec4;

namespace F3DStyle
{

// red with the same saturation (95%) and luminance (50%) as F3D yellow
constexpr int F3D_RED = 0xf94306;
// green with the same saturation (100%) and luminance (74%) as F3D blue
constexpr int F3D_GREEN = 0x7aff7a;
constexpr int F3D_BLUE = 0x788bff;
constexpr int F3D_YELLOW = 0xf9b208;
constexpr int F3D_WHITE = 0xf4f4f4;
// grey between F3D black and F3D white
constexpr int F3D_GREY = 0x545454;
constexpr int F3D_BLACK = 0x141414;

constexpr std::tuple<float, float, float> DecomposeFloatTuple(int val)
{
  return std::make_tuple(
    ((val >> 16) & 0xff) / 255.f, ((val >> 8) & 0xff) / 255.f, (val & 0xff) / 255.f);
}

constexpr std::tuple<float, float, float> GetF3DRed()
{
  return DecomposeFloatTuple(F3D_RED);
}

constexpr std::tuple<float, float, float> GetF3DGreen()
{
  return DecomposeFloatTuple(F3D_GREEN);
}

constexpr std::tuple<float, float, float> GetF3DBlue()
{
  return DecomposeFloatTuple(F3D_BLUE);
}

constexpr std::tuple<float, float, float> GetF3DYellow()
{
  return DecomposeFloatTuple(F3D_YELLOW);
}

constexpr std::tuple<float, float, float> GetF3DWhite()
{
  return DecomposeFloatTuple(F3D_WHITE);
}

constexpr std::tuple<float, float, float> GetF3DGrey()
{
  return DecomposeFloatTuple(F3D_GREY);
}

constexpr std::tuple<float, float, float> GetF3DBlack()
{
  return DecomposeFloatTuple(F3D_BLACK);
}

constexpr float GetDefaultMargin()
{
  return 5.f;
}

#ifdef F3D_MODULE_UI

namespace imgui
{

const ImVec4 GetErrorColor();
const ImVec4 GetWarningColor();
const ImVec4 GetHighlightColor();
const ImVec4 GetCompletionColor();
const ImVec4 GetTextColor();
const ImVec4 GetMidColor();
const ImVec4 GetBackgroundColor();

};
#endif
};

#endif

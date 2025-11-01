#include "F3DStyle.h"

#include <imgui.h>
#include <tuple>

const ImVec4 F3DStyle::GetErrorColor()
{
  auto [r, g, b] = F3DStyle::DecomposeFloatTuple(F3DStyle::F3D_RED);
  return ImVec4{ r, g, b, 1.f };
}

const ImVec4 F3DStyle::GetWarningColor()
{
  auto [r, g, b] = F3DStyle::DecomposeFloatTuple(F3DStyle::F3D_YELLOW);
  return ImVec4{ r, g, b, 1.f };
}

const ImVec4 F3DStyle::GetHighlightColor()
{
  auto [r, g, b] = F3DStyle::DecomposeFloatTuple(F3DStyle::F3D_BLUE);
  return ImVec4{ r, g, b, 1.f };
}

const ImVec4 F3DStyle::GetCompletionColor()
{
  auto [r, g, b] = F3DStyle::DecomposeFloatTuple(F3DStyle::F3D_GREEN);
  return ImVec4{ r, g, b, 1.f };
}

const ImVec4 F3DStyle::GetTextColor()
{
  auto [r, g, b] = F3DStyle::DecomposeFloatTuple(F3DStyle::F3D_WHITE);
  return ImVec4{ r, g, b, 1.f };
}

const ImVec4 F3DStyle::GetMidColor()
{
  auto [r, g, b] = F3DStyle::DecomposeFloatTuple(F3DStyle::F3D_GREY);
  return ImVec4{ r, g, b, 1.f };
}

const ImVec4 F3DStyle::GetBackgroundColor()
{
  auto [r, g, b] = F3DStyle::DecomposeFloatTuple(F3DStyle::F3D_BLACK);
  return ImVec4{ r, g, b, 1.f };
}

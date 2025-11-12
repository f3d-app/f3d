#include "F3DStyle.h"

#ifdef F3D_MODULE_UI

#include <imgui.h>

const inline ImVec4 intToImVec4(int rgb, int alpha = 255)
{
  return ImVec4{
    ((rgb >> 16) & 0xff) / 255.f,
    ((rgb >> 8) & 0xff) / 255.f,
    (rgb & 0xff) / 255.f,
    alpha / 255.f,
  };
}

const ImVec4 F3DStyle::imgui::GetErrorColor()
{
  return intToImVec4(F3DStyle::F3D_RED);
}

const ImVec4 F3DStyle::imgui::GetWarningColor()
{
  return intToImVec4(F3DStyle::F3D_YELLOW);
}

const ImVec4 F3DStyle::imgui::GetHighlightColor()
{
  return intToImVec4(F3DStyle::F3D_BLUE);
}

const ImVec4 F3DStyle::imgui::GetCompletionColor()
{
  return intToImVec4(F3DStyle::F3D_GREEN);
}

const ImVec4 F3DStyle::imgui::GetTextColor()
{
  return intToImVec4(F3DStyle::F3D_WHITE);
}

const ImVec4 F3DStyle::imgui::GetMidColor()
{
  return intToImVec4(F3DStyle::F3D_GREY);
}

const ImVec4 F3DStyle::imgui::GetBackgroundColor()
{
  return intToImVec4(F3DStyle::F3D_BLACK);
}

#endif

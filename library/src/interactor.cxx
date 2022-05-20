#include "interactor.h"

namespace f3d
{
const std::vector<std::pair<std::string, std::string> >& interactor::getDefaultInteractionsInfo()
{
  // clang-format off
  static const std::vector<std::pair<std::string, std::string> > DefaultInteractionsInfo{
    { "C", "Cycle point/cell data coloring" },
    { "S", "Cycle array to color with" },
    { "Y", "Cycle array component to color with" },
    { "B", "Toggle the scalar bar display" },
    { "V", "Toggle volume rendering" },
    { "I", "Toggle inverse volume opacity" },
    { "O", "Toggle point sprites rendering" },
    { "P", "Toggle depth peeling" },
    { "Q", "Toggle SSAO" },
    { "A", "Toggle FXAA" },
    { "T", "Toggle tone mapping" },
    { "E", "Toggle the edges display" },
    { "X", "Toggle the axes display" },
    { "G", "Toggle the grid display" },
    { "N", "Toggle the filename display" },
    { "M", "Toggle the metadata display" },
    { "Z", "Toggle the FPS counter display" },
    { "R", "Toggle raytracing rendering" },
    { "D", "Toggle denoising when raytracing" },
    { "F", "Toggle full screen" },
    { "U", "Toggle blur background" },
    { "K", "Toggle trackball interaction" },
    { "H", "Toggle cheat sheet display" },
    { "?", "Dump camera state to the terminal" },
    { "Escape", "Quit" },
    { "Enter", "Reset camera to initial parameters" },
    { "Space", "Play animation if any" },
    { "Left", "Previous file" },
    { "Right", "Next file" },
    { "Up", "Reload current file" }
  };
  // clang-format on

  return DefaultInteractionsInfo;
};
}

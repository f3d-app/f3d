#include "interactor.h"

namespace f3d
{

//----------------------------------------------------------------------------
interactor& interactor::addDefaultKeyPressInteractions()
{
  this->addKeyPressToggle("b", ModifierKeys::ANY, "ui.bar");
  this->addKeyPressToggle("p", ModifierKeys::ANY, "render.effect.translucency-support");
  this->addKeyPressToggle("q", ModifierKeys::ANY, "render.effect.ambient-occlusion");
  this->addKeyPressToggle("a", ModifierKeys::ANY, "render.effect.anti-aliasing");
  this->addKeyPressToggle("t", ModifierKeys::ANY, "render.effect.tone-mapping");
  this->addKeyPressToggle("e", ModifierKeys::ANY, "render.show-edges");
  this->addKeyPressToggle("x", ModifierKeys::ANY, "interactor.axis");
  this->addKeyPressToggle("g", ModifierKeys::ANY, "render.grid.enable");
  this->addKeyPressToggle("n", ModifierKeys::ANY, "ui.filename");
  this->addKeyPressToggle("m", ModifierKeys::ANY, "ui.metadata");
  this->addKeyPressToggle("r", ModifierKeys::ANY, "render.raytracing.enable");
  this->addKeyPressToggle("d", ModifierKeys::ANY, "render.raytracing.denoise");
  this->addKeyPressToggle("v", ModifierKeys::ANY, "model.volume.enable");
  this->addKeyPressToggle("i", ModifierKeys::ANY, "model.volume.inverse");
  this->addKeyPressToggle("o", ModifierKeys::ANY, "model.point-sprites.enable");
  this->addKeyPressToggle("u", ModifierKeys::ANY, "render.background.blur");
  this->addKeyPressToggle("k", ModifierKeys::ANY, "interactor.trackball");
  this->addKeyPressToggle("f", ModifierKeys::ANY, "render.hdri.ambient");
  this->addKeyPressToggle("f", ModifierKeys::ANY, "render.background.skybox");
  this->addKeyPressToggle("h", ModifierKeys::ANY, "ui.cheatsheet");
  return *this;
}

//----------------------------------------------------------------------------
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
    { "P", "Toggle translucency support" },
    { "Q", "Toggle ambient occlusion" },
    { "A", "Toggle anti-aliasing" },
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
    { "F", "Toggle HDRI ambient lighting" },
    { "J", "Toggle HDRI skybox" },
    { "L", "Increase (+Shift: decrease) lights intensity" },
    { "H", "Toggle cheat sheet display" },
    { "?", "Dump camera state to the terminal" },
    { "1", "Front View camera" },
    { "3", "Left View camera" },
    { "7", "Top View camera" },
    { "9", "Isometric View camera" },
    { "Escape", "Quit" },
    { "Enter", "Reset camera to initial parameters" },
    { "Space", "Play animation if any" },
    { "Left", "Previous file" },
    { "Right", "Next file" },
    { "Up", "Reload current file" },
    { "Down", "Add current file parent directory to the list of files and reload the current file" },
    { "Drop", "Load dropped file, folder or HDRI" }
  };
  // clang-format on

  return DefaultInteractionsInfo;
};
}

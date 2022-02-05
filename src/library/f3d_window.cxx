#include "f3d_window.h"

#include "f3d_options.h"

namespace f3d
{
//----------------------------------------------------------------------------
void window::update(){}

//----------------------------------------------------------------------------
void window::SetOptions(const options* options)
{
  this->Options = options;
}
  
//----------------------------------------------------------------------------
void window::Initialize(bool, std::string){}

//----------------------------------------------------------------------------
vtkF3DRenderer* window::GetRenderer()
{
  return nullptr;
}
};

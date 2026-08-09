#include <F3DStyle.h>

#include <vtkF3DNamedColors.h>
#include <vtkObjectFactory.h>
#include <vtkStdString.h>

vtkStandardNewMacro(vtkF3DNamedColors);

// ----------------------------------------------------------------------------
vtkF3DNamedColors::vtkF3DNamedColors()
{
  this->SetColor(vtkStdString("f3d_red"), vtkColor3ub(F3DStyle::F3D_RED));
  this->SetColor(vtkStdString("f3d_green"), vtkColor3ub(F3DStyle::F3D_GREEN));
  this->SetColor(vtkStdString("f3d_blue"), vtkColor3ub(F3DStyle::F3D_BLUE));
  this->SetColor(vtkStdString("f3d_yellow"), vtkColor3ub(F3DStyle::F3D_YELLOW));
  this->SetColor(vtkStdString("f3d_white"), vtkColor3ub(F3DStyle::F3D_WHITE));
  this->SetColor(vtkStdString("f3d_grey"), vtkColor3ub(F3DStyle::F3D_GREY));
  this->SetColor(vtkStdString("f3d_black"), vtkColor3ub(F3DStyle::F3D_BLACK));
}

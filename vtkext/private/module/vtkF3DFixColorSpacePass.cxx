#include "vtkF3DFixColorSpacePass.h"

#include <vtkObjectFactory.h>
#include <vtkShaderProgram.h>

vtkStandardNewMacro(vtkF3DFixColorSpacePass);

// ----------------------------------------------------------------------------
bool vtkF3DFixColorSpacePass::PostReplaceShaderValues(
  std::string&, std::string&, std::string& fragmentShader, vtkAbstractMapper*, vtkProp*)
{
  vtkShaderProgram::Substitute(
    fragmentShader, "gl_FragData[0].rgb = pow(gl_FragData[0].rgb, vec3(1.0/2.2));", "");
  return true;
}

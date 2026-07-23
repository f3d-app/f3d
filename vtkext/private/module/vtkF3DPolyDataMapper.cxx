#include "vtkF3DPolyDataMapper.h"

#include "F3DLog.h"

#include <vtkActor.h>
#include <vtkDoubleArray.h>
#include <vtkMatrix4x4.h>
#include <vtkObjectFactory.h>
#include <vtkOpenGLRenderWindow.h>
#include <vtkOpenGLRenderer.h>
#include <vtkOpenGLUniforms.h>
#include <vtkOpenGLVertexBufferObject.h>
#include <vtkOpenGLVertexBufferObjectGroup.h>
#include <vtkPointData.h>
#include <vtkPolyData.h>
#include <vtkProperty.h>
#include <vtkShaderProgram.h>
#include <vtkShaderProperty.h>
#include <vtkTexture.h>
#include <vtkUniforms.h>
#include <vtkVersion.h>

#include <regex>

vtkStandardNewMacro(vtkF3DPolyDataMapper);

//-----------------------------------------------------------------------------
void vtkF3DPolyDataMapper::ReplaceShaderValues(
  std::map<vtkShader::Type, vtkShader*> shaders, vtkRenderer* ren, vtkActor* actor)
{
  this->Superclass::ReplaceShaderValues(shaders, ren, actor);

  vtkUniforms* uniforms = actor->GetShaderProperty()->GetVertexCustomUniforms();
  vtkUniforms::TupleType type = uniforms->GetUniformTupleType("jointMatrices");
  if (type != vtkUniforms::TupleTypeInvalid)
  {
    int nbJoints = uniforms->GetUniformNumberOfTuples("jointMatrices");

    // The number of uniform values required by OpenGL is 4096
    // Since a mat4 is 16 values, it means we can only support 256 bones
    // However, there are other uniform values used in practice for other
    // things like materials and we've seen issues starting at 253 bones
    // let's be conservative here and trigger SSBO at 250 bones
    if (nbJoints > 250)
    {
      vtkOpenGLRenderWindow* renWin = vtkOpenGLRenderWindow::SafeDownCast(ren->GetRenderWindow());
      assert(renWin);

      int major, minor;
      renWin->GetOpenGLVersion(major, minor);

      // 4.3 is required for SSBO
      if (major == 4 && minor >= 3)
      {
        // PBR requires linear color space
        auto vertexShader = shaders[vtkShader::Vertex];
        auto VSSource = vertexShader->GetSource();

        std::regex regex("uniform mat4 jointMatrices\\[[0-9]+\\];");
        VSSource = std::regex_replace(VSSource, regex,
          "layout(std430, binding = 0) buffer JointMatrices { mat4 jointMatrices[]; };");

        vtkShaderProgram::Substitute(VSSource, "//VTK::System::Dec", "#version 430");

        vertexShader->SetSource(VSSource);
        this->HasSSBOSkinning = true;
      }
      else
      {
        std::string msg = "A mesh is associated with more than 250 bones (" +
          std::to_string(nbJoints) + "), which requires OpenGL >= 4.3";
        F3DLog::Print(F3DLog::Severity::Warning, msg);

        uniforms->RemoveUniform("jointMatrices");
      }
    }
  }
}

//-----------------------------------------------------------------------------
void vtkF3DPolyDataMapper::SetCustomUniforms(vtkOpenGLHelper& cellBO, vtkActor* actor)
{
  this->Superclass::SetCustomUniforms(cellBO, actor);

  if (this->HasSSBOSkinning)
  {
    vtkUniforms* uniforms = actor->GetShaderProperty()->GetVertexCustomUniforms();

    std::vector<float> buffer(16 * uniforms->GetUniformNumberOfTuples("jointMatrices"));
    uniforms->GetUniformMatrix4x4v("jointMatrices", buffer);

    this->JointMatrices->Upload(buffer, vtkOpenGLBufferObject::ArrayBuffer);
    this->JointMatrices->BindShaderStorage(0);
  }
}

//-----------------------------------------------------------------------------
void vtkF3DPolyDataMapper::ReplaceShaderColor(
  std::map<vtkShader::Type, vtkShader*> shaders, vtkRenderer* ren, vtkActor* actor)
{
  // Fixed in https://gitlab.kitware.com/vtk/vtk/-/merge_requests/13123
#if VTK_VERSION_NUMBER < VTK_VERSION_CHECK(9, 6, 20260411)
  if (actor->GetProperty()->GetInterpolation() == VTK_PBR && !this->DrawingVertices)
  {
    if (this->VBOs->GetNumberOfComponents("scalarColor") != 0 ||
      (this->InterpolateScalarsBeforeMapping && this->ColorCoordinates) ||
      (this->HaveCellScalars && !this->PointPicking))
    {
      // PBR requires linear color space
      auto fragmentShader = shaders[vtkShader::Fragment];
      auto FSSource = fragmentShader->GetSource();

      vtkShaderProgram::Substitute(FSSource, "//VTK::Color::Impl",
        "//VTK::Color::Impl\n"
        "  ambientColor = pow(ambientColor, vec3(2.2));\n"
        "  diffuseColor = pow(diffuseColor, vec3(2.2));\n");
      fragmentShader->SetSource(FSSource);
    }
  }
#endif

  this->Superclass::ReplaceShaderColor(shaders, ren, actor);
}

//-----------------------------------------------------------------------------
void vtkF3DPolyDataMapper::ReplaceShaderLight(
  std::map<vtkShader::Type, vtkShader*> shaders, vtkRenderer* ren, vtkActor* actor)
{
  // Fixed in https://gitlab.kitware.com/vtk/vtk/-/merge_requests/13116
  // which is backported in 9.6.2 in https://gitlab.kitware.com/vtk/vtk/-/merge_requests/13185
#if VTK_VERSION_NUMBER < VTK_VERSION_CHECK(9, 6, 2)
  if (actor->GetProperty()->GetInterpolation() == VTK_PBR &&
    this->PrimitiveInfo[this->LastBoundBO].LastLightComplexity == primitiveInfo::NoLighting)
  {
    // Convert unlit to sRGB
    auto fragmentShader = shaders[vtkShader::Fragment];
    auto FSSource = fragmentShader->GetSource();

    vtkShaderProgram::Substitute(FSSource, "//VTK::Light::Impl",
      "//VTK::Light::Impl\n"
      "  gl_FragData[0].rgb = pow(gl_FragData[0].rgb, vec3(1.0/2.2));\n");
    fragmentShader->SetSource(FSSource);
  }
#endif

  this->Superclass::ReplaceShaderLight(shaders, ren, actor);
}

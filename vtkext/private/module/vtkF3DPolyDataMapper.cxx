#include "vtkF3DPolyDataMapper.h"

#include "F3DLog.h"
#include "vtkF3DRenderer.h"

#include <vtkActor.h>
#include <vtkDoubleArray.h>
#include <vtkImageData.h>
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
#include <vtkUnsignedCharArray.h>
#include <vtkVersion.h>

#include <array>
#include <cmath>
#include <regex>

vtkStandardNewMacro(vtkF3DPolyDataMapper);

//-----------------------------------------------------------------------------
void vtkF3DPolyDataMapper::SetUseLinearColorSpace(bool use)
{
  if (this->UseLinearColorSpace != use)
  {
    this->UseLinearColorSpace = use;
    this->ClearColorArrays();
    this->LinearColorTextureMap = nullptr;
    this->Modified();
  }
}

//-----------------------------------------------------------------------------
vtkUnsignedCharArray* vtkF3DPolyDataMapper::MapScalars(
  vtkDataSet* input, double alpha, int& cellFlag)
{
  // Populates the vtkMapper::Colors with gamma corrected sRGB.
  this->Superclass::MapScalars(input, alpha, cellFlag);
  if (!this->UseLinearColorSpace)
  {
    return this->Colors;
  }

  if (this->ColorTextureMap && this->ColorTextureMap != this->LinearColorTextureMap)
  {
    const vtkUnsignedCharArray* source =
      vtkUnsignedCharArray::SafeDownCast(this->ColorTextureMap->GetPointData()->GetScalars());
    this->LinearColorTextureMap = vtkSmartPointer<vtkImageData>::New();
    this->LinearColorTextureMap->CopyStructure(this->ColorTextureMap);
    this->LinearColorTextureMap->AllocateScalars(source->GetDataType(), 4);
    vtkUnsignedCharArray* destination =
      vtkUnsignedCharArray::SafeDownCast(this->LinearColorTextureMap->GetPointData()->GetScalars());
    for (vtkIdType i = 0; i < source->GetNumberOfTuples(); ++i)
    {
      for (int component = 0; component < 4; ++component)
      {
        const unsigned char value = source->GetTypedComponent(i, component);
        // Approx. color components converted to linear except alpha channel.
        destination->SetTypedComponent(i, component,
          component < 3 ? static_cast<unsigned char>(std::pow(value / 255.0, 2.2) * 255.0) : value);
      }
    }
    this->ColorTextureMap->UnRegister(this);
    this->ColorTextureMap = this->LinearColorTextureMap;
    this->ColorTextureMap->Register(this);
  }
  else if (!this->ColorTextureMap)
  {
    this->LinearColorTextureMap = nullptr;
  }

  return this->Colors;
}

//-----------------------------------------------------------------------------
void vtkF3DPolyDataMapper::ReplaceShaderValues(
  std::map<vtkShader::Type, vtkShader*> shaders, vtkRenderer* ren, vtkActor* actor)
{
  this->Superclass::ReplaceShaderValues(shaders, ren, actor);

  vtkUniforms* uniforms = actor->GetShaderProperty()->GetVertexCustomUniforms();
  const vtkUniforms::TupleType type = uniforms->GetUniformTupleType("jointMatrices");
  if (type != vtkUniforms::TupleTypeInvalid)
  {
    const int nbJoints = uniforms->GetUniformNumberOfTuples("jointMatrices");

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

        const std::regex regex("uniform mat4 jointMatrices\\[[0-9]+\\];");
        VSSource = std::regex_replace(VSSource, regex,
          "layout(std430, binding = 0) buffer JointMatrices { mat4 jointMatrices[]; };");

        vtkShaderProgram::Substitute(VSSource, "//VTK::System::Dec", "#version 430");

        vertexShader->SetSource(VSSource);
        this->HasSSBOSkinning = true;
      }
      else
      {
        const std::string msg = "A mesh is associated with more than 250 bones (" +
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

#if VTK_VERSION_NUMBER >= VTK_VERSION_CHECK(9, 7, 20260730)
//-----------------------------------------------------------------------------
bool vtkF3DPolyDataMapper::FragmentShaderUsesPrimitiveID(vtkRenderer* renderer, vtkActor* actor)
{
  bool usesPrimitiveID = this->Superclass::FragmentShaderUsesPrimitiveID(renderer, actor);

  // https://gitlab.kitware.com/vtk/vtk/-/merge_requests/13207
  // Enable Mesa workaround when using Stochastic blending because it uses gl_PrimitiveID
  // see vtkext/private/module/vtkF3DStochasticTransparentPass.cxx
  const vtkF3DRenderer* ren = vtkF3DRenderer::SafeDownCast(renderer);
  if (ren)
  {
    usesPrimitiveID =
      usesPrimitiveID || ren->GetBlendingMode() == vtkF3DRenderer::BlendingMode::STOCHASTIC;
  }
  return usesPrimitiveID;
}
#endif

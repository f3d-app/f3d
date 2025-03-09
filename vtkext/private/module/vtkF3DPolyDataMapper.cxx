#include "vtkF3DPolyDataMapper.h"

#include "F3DLog.h"

#include <vtkActor.h>
#include <vtkDoubleArray.h>
#include <vtkMatrix4x4.h>
#include <vtkObjectFactory.h>
#include <vtkOpenGLRenderWindow.h>
#include <vtkOpenGLRenderer.h>
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

vtkStandardNewMacro(vtkF3DPolyDataMapper);

//-----------------------------------------------------------------------------
vtkF3DPolyDataMapper::vtkF3DPolyDataMapper()
{
#if VTK_VERSION_NUMBER >= VTK_VERSION_CHECK(9, 2, 20230518)
  this->SetVBOShiftScaleMethod(vtkPolyDataMapper::ShiftScaleMethodType::DISABLE_SHIFT_SCALE);
#else
  this->SetVBOShiftScaleMethod(vtkOpenGLVertexBufferObject::DISABLE_SHIFT_SCALE);
#endif

  // map glTF arrays to GPU VBOs
  this->MapDataArrayToVertexAttribute(
    "weights", "WEIGHTS_0", vtkDataObject::FIELD_ASSOCIATION_POINTS);
  this->MapDataArrayToVertexAttribute(
    "joints", "JOINTS_0", vtkDataObject::FIELD_ASSOCIATION_POINTS);

  // morph targets
  // OpenGL limits the input attributes to 16 vectors
  // We ignore morphing on tangent on purpose to maximize the number of targets we
  // can support
  for (int i = 0; i < 4; i++)
  {
    std::string name = "target" + std::to_string(i) + "_position";
    this->MapDataArrayToVertexAttribute(
      name.c_str(), name.c_str(), vtkDataObject::FIELD_ASSOCIATION_POINTS);
    name = "target" + std::to_string(i) + "_normal";
    this->MapDataArrayToVertexAttribute(
      name.c_str(), name.c_str(), vtkDataObject::FIELD_ASSOCIATION_POINTS);
  }
}

//-----------------------------------------------------------------------------
void vtkF3DPolyDataMapper::ReplaceShaderValues(
  std::map<vtkShader::Type, vtkShader*> shaders, vtkRenderer* ren, vtkActor* actor)
{
  auto vertexShader = shaders[vtkShader::Vertex];
  auto VSSource = vertexShader->GetSource();

  bool hasTangents =
    this->VBOs->GetNumberOfComponents("tangentMC") == 3 && actor->GetProperty()->GetLighting();
  bool hasNormals =
    this->VBOs->GetNumberOfComponents("normalMC") == 3 && actor->GetProperty()->GetLighting();

  vtkUniforms* uniforms = actor->GetShaderProperty()->GetVertexCustomUniforms();

#if VTK_VERSION_NUMBER >= VTK_VERSION_CHECK(9, 4, 20250218)
  using f3d_texinfo = vtkOpenGLPolyDataMapper::texinfo;
#else
  using f3d_texinfo = std::pair<vtkTexture*, std::string>;
#endif
  std::vector<f3d_texinfo> textures = this->GetTextures(actor);
  hasTangents = hasTangents &&
    std::find_if(textures.begin(), textures.end(),
      [](const f3d_texinfo& tex) { return tex.second == "normalTex"; }) != textures.end();

  std::string customDecl = "//VTK::CustomUniforms::Dec\n";
  std::string beginImpl;
  std::string posImpl = "  vec4 posMC = vertexMC;\n";

  // normals and tangents can be modified by skinnning and morphing in model space
  std::string normalImpl;
  if (hasNormals)
  {
    normalImpl += "  normalVCVSOutput = normalMC;\n";
  }
  if (hasTangents)
  {
    normalImpl += "  tangentVCVSOutput = tangentMC;\n";
  }

  // morphing
  vtkUniforms::TupleType type = uniforms->GetUniformTupleType("morphWeights");
  if (type != vtkUniforms::TupleTypeInvalid)
  {
    for (int i = 0; i < 4; i++)
    {
      std::string name = "target" + std::to_string(i) + "_position";

      // modify position using morph weights
      if (this->VBOs->GetNumberOfComponents(name.c_str()) == 3)
      {
        customDecl += "in vec3 ";
        customDecl += name;
        customDecl += ";\n";

        posImpl += " posMC += morphWeights[";
        posImpl += std::to_string(i);
        posImpl += "] * vec4(";
        posImpl += name;
        posImpl += ", 0);\n";
      }

      name = "target" + std::to_string(i) + "_normal";

      // modify normal using morph weights
      if (this->VBOs->GetNumberOfComponents(name.c_str()) == 3)
      {
        customDecl += "in vec3 ";
        customDecl += name;
        customDecl += ";\n";

        if (hasNormals)
        {
          normalImpl += " normalVCVSOutput += morphWeights[";
          normalImpl += std::to_string(i);
          normalImpl += "] * ";
          normalImpl += name;
          normalImpl += ";\n";
        }
      }
    }
  }

  // skin
  type = uniforms->GetUniformTupleType("jointMatrices");
  if (type != vtkUniforms::TupleTypeInvalid)
  {
    int nbJoints = uniforms->GetUniformNumberOfTuples("jointMatrices");

    bool skinningSupported = true;

    // The number of uniform values required by OpenGL is 4096
    // Since a mat4 is 16 values, it means we can only support 256 bones
    // However, there are other uniform values used in practice for other
    // things like materials and we've seen issues starting at 253 bones
    // let's be conservative here and trigger SSBO at 250 bones
    if (nbJoints > 250)
    {
#if VTK_VERSION_NUMBER >= VTK_VERSION_CHECK(9, 3, 20231108)
      vtkOpenGLRenderWindow* renWin = vtkOpenGLRenderWindow::SafeDownCast(ren->GetRenderWindow());
      assert(renWin);

      int major, minor;
      renWin->GetOpenGLVersion(major, minor);

      // 4.3 is required for SSBO
      if (major == 4 && minor >= 3)
      {
        std::vector<float> buffer(16 * sizeof(float) * nbJoints);
        uniforms->GetUniformMatrix4x4v("jointMatrices", buffer);

        this->JointMatrices->Upload(buffer, vtkOpenGLBufferObject::ArrayBuffer);
        this->JointMatrices->BindShaderStorage(0);

        uniforms->RemoveUniform("jointMatrices");

        customDecl += "layout(std430, binding = 0) readonly buffer JointsData\n"
                      "{\n"
                      "  mat4 jointMatrices[];\n"
                      "};\n";

        vtkShaderProgram::Substitute(VSSource, "//VTK::System::Dec", "#version 430");
      }
      else
      {
        std::string msg = "A mesh is associated with more than 250 bones (" +
          std::to_string(nbJoints) + "), which requires OpenGL >= 4.3";
        F3DLog::Print(F3DLog::Severity::Warning, msg);

        skinningSupported = false;
      }
#else
      std::string msg = "A mesh is associated with more than 250 bones (" +
        std::to_string(nbJoints) + "), which is not supported by VTK < 9.3.20231108";
      F3DLog::Print(F3DLog::Severity::Warning, msg);

      skinningSupported = false;
#endif
    }

    if (skinningSupported)
    {
      customDecl += "in vec4 joints;\n"
                    "in vec4 weights;\n";

      // compute skinning matrix with current uniform weights
      beginImpl += "  mat4 skinMat = weights.x * jointMatrices[int(joints.x)]\n"
                   "               + weights.y * jointMatrices[int(joints.y)]\n"
                   "               + weights.z * jointMatrices[int(joints.z)]\n"
                   "               + weights.w * jointMatrices[int(joints.w)];\n";

      posImpl += "  posMC = skinMat * posMC;\n";

      // apply the matrix to normals and tangents
      if (hasNormals)
      {
        normalImpl += "  normalVCVSOutput = mat3(skinMat) * normalVCVSOutput;\n";
      }
      if (hasTangents)
      {
        normalImpl += "  tangentVCVSOutput = mat3(skinMat) * tangentVCVSOutput;\n";
      }
    }
  }

  posImpl += "  gl_Position = MCDCMatrix * posMC;\n";

  if (this->PrimitiveInfo[this->LastBoundBO].LastLightComplexity > 0)
  {
    posImpl += "  vertexVCVSOutput = MCVCMatrix * posMC;\n";
  }

  if (hasNormals)
  {
    normalImpl += "  normalVCVSOutput = normalMatrix * normalVCVSOutput;\n";
  }
  if (hasTangents)
  {
    normalImpl += "  tangentVCVSOutput = normalMatrix * tangentVCVSOutput;\n";
  }

  vtkShaderProgram::Substitute(VSSource, "//VTK::CustomUniforms::Dec", customDecl);
  vtkShaderProgram::Substitute(VSSource, "//VTK::PositionVC::Impl", posImpl);
  vtkShaderProgram::Substitute(VSSource, "//VTK::Normal::Impl", normalImpl);
  vtkShaderProgram::Substitute(VSSource, "//VTK::CustomBegin::Impl", beginImpl);

  vertexShader->SetSource(VSSource);

  this->Superclass::ReplaceShaderValues(shaders, ren, actor);
}

//-----------------------------------------------------------------------------
bool vtkF3DPolyDataMapper::RenderWithMatCap(vtkActor* actor)
{
  // check if we have normals
  if (this->VBOs->GetNumberOfComponents("normalMC") != 3)
  {
    return false;
  }

  auto textures = actor->GetProperty()->GetAllTextures();
  auto fn = [](const std::pair<std::string, vtkTexture*>& tex) { return tex.first == "matcap"; };
  return std::find_if(textures.begin(), textures.end(), fn) != textures.end();
}

//-----------------------------------------------------------------------------
void vtkF3DPolyDataMapper::ReplaceShaderColor(
  std::map<vtkShader::Type, vtkShader*> shaders, vtkRenderer* ren, vtkActor* actor)
{
  if (this->RenderWithMatCap(actor))
  {
    auto fragmentShader = shaders[vtkShader::Fragment];
    auto FSSource = fragmentShader->GetSource();

    std::string customLight = "//VTK::Color::Impl\n"
                              "vec2 uv = vec2(normalVCVSOutput.xy) * 0.5 + vec2(0.5,0.5);\n"
                              "vec3 diffuseColor = vec3(0.0);\n"
                              "vec3 ambientColor = texture(matcap, uv).rgb;\n";

    vtkShaderProgram::Substitute(FSSource, "//VTK::Color::Impl", customLight);
    fragmentShader->SetSource(FSSource);
  }
  else
  {
    this->Superclass::ReplaceShaderColor(shaders, ren, actor);
  }
}

//-----------------------------------------------------------------------------
void vtkF3DPolyDataMapper::ReplaceShaderLight(
  std::map<vtkShader::Type, vtkShader*> shaders, vtkRenderer* ren, vtkActor* actor)
{
  if (this->RenderWithMatCap(actor))
  {
    auto fragmentShader = shaders[vtkShader::Fragment];
    auto FSSource = fragmentShader->GetSource();

    // set final color to gamma-corrected ambient color
    std::string customLight = "//VTK::Light::Impl\n"
                              "gl_FragData[0] = vec4(pow(ambientColor, vec3(1.0/2.2)), 1.0);\n";

    vtkShaderProgram::Substitute(FSSource, "//VTK::Light::Impl", customLight);
    fragmentShader->SetSource(FSSource);
  }
  else
  {
    this->Superclass::ReplaceShaderLight(shaders, ren, actor);
  }
}

//-----------------------------------------------------------------------------
void vtkF3DPolyDataMapper::ReplaceShaderTCoord(
  std::map<vtkShader::Type, vtkShader*> shaders, vtkRenderer* ren, vtkActor* actor)
{
  if (this->RenderWithMatCap(actor))
  {
    // disable default behavior of VTK with textures to avoid blending with itself
    auto fragmentShader = shaders[vtkShader::Fragment];
    std::string FSSource = fragmentShader->GetSource();

    vtkShaderProgram::Substitute(FSSource, "//VTK::TCoord::Impl", "");

    fragmentShader->SetSource(FSSource);
  }

  this->Superclass::ReplaceShaderTCoord(shaders, ren, actor);
}

#if VTK_VERSION_NUMBER < VTK_VERSION_CHECK(9, 3, 20230902)
// Integrated in VTK in https://gitlab.kitware.com/vtk/vtk/-/merge_requests/10456
//------------------------------------------------------------------------------
bool vtkF3DPolyDataMapper::GetNeedToRebuildShaders(
  vtkOpenGLHelper& cellBO, vtkRenderer* ren, vtkActor* actor)
{
  bool ret = this->Superclass::GetNeedToRebuildShaders(cellBO, ren, actor);

  vtkOpenGLRenderer* oren = static_cast<vtkOpenGLRenderer*>(ren);
  vtkTexture* envTexture = oren->GetEnvironmentTexture();
  if (this->EnvTexture != envTexture ||
    (envTexture && envTexture->GetMTime() > this->EnvTextureTime))
  {
    ret = true;
    this->EnvTexture = envTexture;
    if (envTexture)
    {
      this->EnvTextureTime = envTexture->GetMTime();
    }
  }

  return ret;
}
#endif

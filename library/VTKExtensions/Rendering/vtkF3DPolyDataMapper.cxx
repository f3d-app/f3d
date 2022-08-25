#include "vtkF3DPolyDataMapper.h"

#include <vtkActor.h>
#include <vtkDoubleArray.h>
#include <vtkMatrix4x4.h>
#include <vtkObjectFactory.h>
#include <vtkOpenGLVertexBufferObject.h>
#include <vtkOpenGLVertexBufferObjectGroup.h>
#include <vtkPointData.h>
#include <vtkPolyData.h>
#include <vtkProperty.h>
#include <vtkShaderProgram.h>
#include <vtkShaderProperty.h>
#include <vtkUniforms.h>
#include <vtkVersion.h>

vtkStandardNewMacro(vtkF3DPolyDataMapper);

//-----------------------------------------------------------------------------
vtkF3DPolyDataMapper::vtkF3DPolyDataMapper()
{
  this->SetVBOShiftScaleMethod(vtkOpenGLVertexBufferObject::DISABLE_SHIFT_SCALE);

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

  using texinfo = std::pair<vtkTexture*, std::string>;
  std::vector<texinfo> textures = this->GetTextures(actor);
  hasTangents = hasTangents &&
    std::find_if(textures.begin(), textures.end(),
      [](const texinfo& tex) { return tex.second == "normalTex"; }) != textures.end();

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

  posImpl += "  gl_Position = MCDCMatrix * posMC;\n";

// LastLightComplexity Changed in https://gitlab.kitware.com/vtk/vtk/-/merge_requests/7933
#if VTK_VERSION_NUMBER >= VTK_VERSION_CHECK(9, 0, 20210506)
  if (this->PrimitiveInfo[this->LastBoundBO].LastLightComplexity > 0)
#else
  if (this->LastLightComplexity[this->LastBoundBO] > 0)
#endif
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

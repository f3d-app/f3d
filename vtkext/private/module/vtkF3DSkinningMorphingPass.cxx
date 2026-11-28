#include "vtkF3DSkinningMorphingPass.h"

#include <vtkActor.h>
#include <vtkDataObject.h>
#include <vtkInformation.h>
#include <vtkObjectFactory.h>
#include <vtkOpenGLRenderer.h>
#include <vtkPointData.h>
#include <vtkPolyData.h>
#include <vtkPolyDataMapper.h>
#include <vtkProp.h>
#include <vtkProperty.h>
#include <vtkRenderState.h>
#include <vtkShaderProgram.h>
#include <vtkShaderProperty.h>
#include <vtkUniforms.h>

vtkStandardNewMacro(vtkF3DSkinningMorphingPass);

// ----------------------------------------------------------------------------
void vtkF3DSkinningMorphingPass::MapAttributes(vtkPolyDataMapper* polyMapper, vtkPolyData* input)
{
  // skinning
  if (input->GetPointData()->GetArray("WEIGHTS_0") != nullptr &&
    input->GetPointData()->GetArray("JOINTS_0") != nullptr)
  {
    // map glTF arrays to GPU VBOs
    polyMapper->MapDataArrayToVertexAttribute(
      "weights", "WEIGHTS_0", vtkDataObject::FIELD_ASSOCIATION_POINTS);
    polyMapper->MapDataArrayToVertexAttribute(
      "joints", "JOINTS_0", vtkDataObject::FIELD_ASSOCIATION_POINTS);
  }

  // morph targets
  // OpenGL limits the input attributes to 16 vectors
  // We ignore morphing on tangent on purpose to maximize the number of targets we
  // can support
  for (int j = 0; j < 4; j++)
  {
    const std::string namePosition = "target" + std::to_string(j) + "_position";

    if (input->GetPointData()->GetArray(namePosition.c_str()) == nullptr)
    {
      break;
    }

#ifdef F3D_USE_GLES
    // with GLES, we append all morphing targets into a single texture
    polyMapper->MapDataArrayToVertexAttribute(
      "target_position", namePosition.c_str(), vtkDataObject::FIELD_ASSOCIATION_POINTS);
#else
    polyMapper->MapDataArrayToVertexAttribute(
      namePosition.c_str(), namePosition.c_str(), vtkDataObject::FIELD_ASSOCIATION_POINTS);
#endif

    const std::string nameNormal = "target" + std::to_string(j) + "_normal";
    if (input->GetPointData()->GetArray(nameNormal.c_str()) != nullptr)
    {
#ifdef F3D_USE_GLES
      polyMapper->MapDataArrayToVertexAttribute(
        "target_normal", nameNormal.c_str(), vtkDataObject::FIELD_ASSOCIATION_POINTS);
#else
      polyMapper->MapDataArrayToVertexAttribute(
        nameNormal.c_str(), nameNormal.c_str(), vtkDataObject::FIELD_ASSOCIATION_POINTS);
#endif
    }
  }
}

// ----------------------------------------------------------------------------
void vtkF3DSkinningMorphingPass::Render(const vtkRenderState* s)
{
  vtkOpenGLRenderer* glRenderer = vtkOpenGLRenderer::SafeDownCast(s->GetRenderer());
  this->LightComplexity = glRenderer ? glRenderer->GetLightingComplexity() : 0;

  vtkProp* const* props = s->GetPropArray();
  for (int i = 0; i < s->GetPropArrayCount(); i++)
  {
    vtkActor* actor = vtkActor::SafeDownCast(props[i]);
    vtkPolyDataMapper* polyMapper =
      actor ? vtkPolyDataMapper::SafeDownCast(actor->GetMapper()) : nullptr;
    vtkPolyData* input =
      polyMapper && polyMapper->GetNumberOfInputPorts() > 0 ? polyMapper->GetInput() : nullptr;
    if (input)
    {
      this->MapAttributes(polyMapper, input);
    }
  }

  this->Superclass::Render(s);
}

// ----------------------------------------------------------------------------
void vtkF3DSkinningMorphingPass::ReplaceSkinningMorphing(
  std::string& vertexShader, vtkActor* actor, vtkPolyData* polyData)
{
  if (polyData)
  {
    vtkUniforms* uniforms = actor->GetShaderProperty()->GetVertexCustomUniforms();
    const bool hasMorphing =
      uniforms->GetUniformTupleType("morphWeights") != vtkUniforms::TupleTypeInvalid;
    const bool hasSkinning =
      uniforms->GetUniformTupleType("jointMatrices") != vtkUniforms::TupleTypeInvalid;

    if (hasMorphing || hasSkinning)
    {
      bool hasTangents =
        polyData->GetPointData()->GetTangents() && actor->GetProperty()->GetLighting();
      const bool hasNormals =
        polyData->GetPointData()->GetNormals() && actor->GetProperty()->GetLighting();
      hasTangents = hasTangents && (actor->GetProperty()->GetTexture("normalTex") != nullptr);

      std::string customDecl = "//VTK::CustomUniforms::Dec\n";
      std::string beginImpl = "//VTK::CustomBegin::Impl\n";
      std::string posImpl = "//VTK::PositionVC::Impl\n"
                            "  vec4 posMC = vertexMC;\n";

      // normals and tangents can be modified by skinnning and morphing in model space
      std::string normalImpl = "//VTK::Normal::Impl\n";
      if (hasNormals)
      {
        normalImpl += "  normalVCVSOutput = normalMC;\n";
      }
      if (hasTangents)
      {
#ifdef F3D_USE_GLES
        normalImpl += "  tangentVCVS = tangentMC;\n";
#else
        normalImpl += "  tangentVCVSOutput = tangentMC;\n";
#endif
      }

      // morphing
      if (hasMorphing)
      {
#ifdef F3D_USE_GLES
        customDecl += "uniform sampler2D target_position;\n";
        customDecl += "uniform sampler2D target_normal;\n";
#endif

        for (int i = 0; i < 4; i++)
        {
          std::string name = "target" + std::to_string(i) + "_position";

          // modify position using morph weights
          if (polyData->GetPointData()->GetArray(name.c_str()) != nullptr)
          {
#ifdef F3D_USE_GLES
            posImpl += " posMC += morphWeights[";
            posImpl += std::to_string(i);
            posImpl += "] * vec4(texelFetchBuffer(target_position, ";
            posImpl += std::to_string(i);
            posImpl += " * pointCount + pointId).xyz, 0);\n";
#else
            customDecl += "in vec3 ";
            customDecl += name;
            customDecl += ";\n";

            posImpl += " posMC += morphWeights[";
            posImpl += std::to_string(i);
            posImpl += "] * vec4(";
            posImpl += name;
            posImpl += ", 0);\n";
#endif
          }

          name = "target" + std::to_string(i) + "_normal";

          // modify normal using morph weights
          if (polyData->GetPointData()->GetArray(name.c_str()) != nullptr)
          {
#ifdef F3D_USE_GLES
            if (hasNormals)
            {
              normalImpl += " normalVCVSOutput += morphWeights[";
              normalImpl += std::to_string(i);
              normalImpl += "] * texelFetchBuffer(target_normal, ";
              normalImpl += std::to_string(i);
              normalImpl += " * pointCount + pointId).xyz;\n";
            }
#else
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
#endif
          }
        }
      }

      // skin
      if (hasSkinning)
      {
        // automatically added with GLES
#ifdef F3D_USE_GLES
        customDecl += "uniform mediump usampler2D joints;\n"
                      "uniform mediump sampler2D weights;\n";
#else
        customDecl += "in vec4 joints;\n"
                      "in vec4 weights;\n";
#endif

#ifdef F3D_USE_GLES
        beginImpl += "  vec4 currentWeight = texelFetchBuffer(weights, pointId);\n"
                     "  uvec4 currentJoints = texelFetchBuffer(joints, pointId);\n";
#else
        beginImpl += "  vec4 currentWeight = weights;\n"
                     "  ivec4 currentJoints = ivec4(joints);\n";
#endif

        // compute skinning matrix with current uniform weights
        beginImpl += "  mat4 skinMat = currentWeight.x * jointMatrices[currentJoints.x]\n"
                     "               + currentWeight.y * jointMatrices[currentJoints.y]\n"
                     "               + currentWeight.z * jointMatrices[currentJoints.z]\n"
                     "               + currentWeight.w * jointMatrices[currentJoints.w];\n";

        posImpl += "  posMC = skinMat * posMC;\n";

        // apply the matrix to normals and tangents
        if (hasNormals)
        {
          normalImpl += "  normalVCVSOutput = mat3(skinMat) * normalVCVSOutput;\n";
        }
        if (hasTangents)
        {
#ifdef F3D_USE_GLES
          normalImpl += "  tangentVCVS = mat3(skinMat) * tangentVCVS;\n";
#else
          normalImpl += "  tangentVCVSOutput = mat3(skinMat) * tangentVCVSOutput;\n";
#endif
        }
      }

      posImpl += "  gl_Position = MCDCMatrix * posMC;\n";

      if (this->LightComplexity > 0)
      {
        posImpl += "  vertexVCVSOutput = MCVCMatrix * posMC;\n";
      }

      if (hasNormals)
      {
        normalImpl += "  normalVCVSOutput = normalMatrix * normalVCVSOutput;\n";
#ifdef F3D_USE_GLES
        // for some reason GLES shader has two normal outputs
        normalImpl += "  normalVCVSInput = normalVCVSOutput;\n";
#endif
      }
      if (hasTangents)
      {
#ifdef F3D_USE_GLES
        normalImpl += "  tangentVCVS = normalMatrix * tangentVCVS;\n";
#else
        normalImpl += "  tangentVCVSOutput = normalMatrix * tangentVCVSOutput;\n";
#endif
      }

      vtkShaderProgram::Substitute(vertexShader, "//VTK::CustomUniforms::Dec", customDecl);
      vtkShaderProgram::Substitute(vertexShader, "//VTK::PositionVC::Impl", posImpl);
      vtkShaderProgram::Substitute(vertexShader, "//VTK::Normal::Impl", normalImpl);
      vtkShaderProgram::Substitute(vertexShader, "//VTK::CustomBegin::Impl", beginImpl);
    }
  }
}

// ----------------------------------------------------------------------------
bool vtkF3DSkinningMorphingPass::PreReplaceShaderValues(
  std::string& vertexShader, std::string&, std::string&, vtkAbstractMapper* mapper, vtkProp* prop)
{
  vtkPolyDataMapper* polyMapper = vtkPolyDataMapper::SafeDownCast(mapper);
  vtkActor* actor = vtkActor::SafeDownCast(prop);

  if (!polyMapper || !actor)
  {
    return true;
  }

  vtkPolyData* input = polyMapper->GetNumberOfInputPorts() > 0 ? polyMapper->GetInput() : nullptr;

  this->ReplaceSkinningMorphing(vertexShader, actor, input);

  return true;
}

// ----------------------------------------------------------------------------
bool vtkF3DSkinningMorphingPass::PostReplaceShaderValues([[maybe_unused]] std::string& vertexShader,
  std::string&, std::string&, [[maybe_unused]] vtkAbstractMapper* mapper,
  [[maybe_unused]] vtkProp* prop)
{
#ifdef F3D_USE_GLES
  vtkPolyDataMapper* polyMapper = vtkPolyDataMapper::SafeDownCast(mapper);
  vtkActor* actor = vtkActor::SafeDownCast(prop);

  if (!polyMapper || !actor)
  {
    return true;
  }

  vtkPolyData* polyData =
    polyMapper->GetNumberOfInputPorts() > 0 ? polyMapper->GetInput() : nullptr;
  if (polyData)
  {
    vtkUniforms* uniforms = actor->GetShaderProperty()->GetVertexCustomUniforms();
    bool hasMorphing =
      uniforms->GetUniformTupleType("morphWeights") != vtkUniforms::TupleTypeInvalid;
    bool hasSkinning =
      uniforms->GetUniformTupleType("jointMatrices") != vtkUniforms::TupleTypeInvalid;

    if (!hasMorphing && !hasSkinning)
    {
      return true;
    }

    for (int i = 0; i < 2; i++)
    {
      std::string index = std::to_string(i);
      std::string positionLine =
        "vec4 p" + index + "MC = vec4(fetchTuple3(positions, p" + index + "), 1.0);\n";

      std::string replacementLines = positionLine;

      if (hasMorphing)
      {
        for (int j = 0; j < 4; j++)
        {
          std::string namePosition = "target" + std::to_string(j) + "_position";

          if (polyData->GetPointData()->GetArray(namePosition.c_str()) == nullptr)
          {
            break;
          }

          replacementLines += "  p" + index + "MC += morphWeights[" + std::to_string(j) +
            "] * vec4(texelFetchBuffer(target_position, " + std::to_string(j) +
            " * pointCount + p" + index + ").xyz, 0);\n";
        }
      }

      if (hasSkinning)
      {
        // clang-format off
        replacementLines +=
          "  vec4 currentWeight" + index + " = texelFetchBuffer(weights, p" + index + ");\n"
          "  uvec4 currentJoints" + index + " = texelFetchBuffer(joints, p" + index + ");\n"
          "  mat4 skinMat" + index + " = currentWeight" + index + ".x * jointMatrices[currentJoints" + index + ".x]\n"
          "             + currentWeight" + index + ".y * jointMatrices[currentJoints" + index + ".y]\n"
          "             + currentWeight" + index + ".z * jointMatrices[currentJoints" + index + ".z]\n"
          "             + currentWeight" + index + ".w * jointMatrices[currentJoints" + index + ".w];\n"
          "  p" + index + "MC = skinMat" + index + " * p" + index + "MC;\n";
        // clang-format on
      }

      vtkShaderProgram::Substitute(vertexShader, positionLine, replacementLines);
    }
  }
#endif
  return true;
}

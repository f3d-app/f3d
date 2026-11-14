#include "vtkF3DPolyDataMapper.h"

#include "F3DLog.h"
#include "vtkF3DRenderer.h"

#include <vtkActor.h>
#include <vtkBoundingBox.h>
#include <vtkDoubleArray.h>
#include <vtkFieldData.h>
#include <vtkFloatArray.h>
#include <vtkImageData.h>
#include <vtkIntArray.h>
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
#include <vtk_glad.h>

#include <algorithm>
#include <cmath>
#include <regex>

vtkStandardNewMacro(vtkF3DPolyDataMapper);

//-----------------------------------------------------------------------------
double* vtkF3DPolyDataMapper::GetBounds()
{
  this->Superclass::GetBounds();
  auto* data = this->GetInput();
  if (!data)
  {
    return this->Bounds;
  }
  auto* ranges = vtkIntArray::SafeDownCast(data->GetPointData()->GetArray("blendShapeRanges"));
  auto* positions =
    vtkFloatArray::SafeDownCast(data->GetPointData()->GetArray("blendShapeBindPositions"));
  auto* offsets = vtkFloatArray::SafeDownCast(data->GetFieldData()->GetArray("blendShapeOffsets"));
  auto* weights = vtkFloatArray::SafeDownCast(data->GetFieldData()->GetArray("blendShapeWeights"));
  if (!ranges || !positions || !offsets || !weights || ranges->GetNumberOfComponents() != 2 ||
    positions->GetNumberOfComponents() != 3 || offsets->GetNumberOfComponents() != 4 ||
    weights->GetNumberOfComponents() != 1 ||
    ranges->GetNumberOfTuples() != data->GetNumberOfPoints() ||
    positions->GetNumberOfTuples() != data->GetNumberOfPoints())
  {
    return this->Bounds;
  }
  const vtkMTimeType modified = std::max({ data->GetMTime(), ranges->GetMTime(),
    positions->GetMTime(), offsets->GetMTime(), weights->GetMTime() });
  if (this->BoundsInput == data && this->MorphingBoundsTime > modified)
  {
    return this->MorphingBounds;
  }
  vtkBoundingBox box;
  for (vtkIdType i = 0; i < positions->GetNumberOfTuples(); ++i)
  {
    int range[2] = {};
    ranges->GetTypedTuple(i, range);
    if (range[0] < 0 || range[1] < range[0] || range[1] > offsets->GetNumberOfTuples())
    {
      return this->Bounds;
    }
    double point[3];
    positions->GetTuple(i, point);
    for (int j = range[0]; j < range[1]; ++j)
    {
      float offset[4] = {};
      offsets->GetTypedTuple(j, offset);
      if (!std::isfinite(offset[3]) || offset[3] < 0 || offset[3] >= weights->GetNumberOfValues() ||
        std::floor(offset[3]) != offset[3])
      {
        return this->Bounds;
      }
      const double weight = weights->GetValue(static_cast<vtkIdType>(offset[3]));
      for (int component = 0; component < 3; ++component)
      {
        point[component] += offset[component] * weight;
      }
    }
    box.AddPoint(point);
  }
  box.GetBounds(this->MorphingBounds);
  this->BoundsInput = data;
  this->MorphingBoundsTime.Modified();
  return this->MorphingBounds;
}

//-----------------------------------------------------------------------------
bool vtkF3DPolyDataMapper::PrepareSparseMorphing(
  vtkOpenGLRenderWindow* window, vtkPolyData* polydata)
{
  bool enabled = false;
#ifndef F3D_USE_GLES
  int major = 0, minor = 0;
  if (window && polydata)
  {
    window->GetOpenGLVersion(major, minor);
  }
  if (major > 4 || (major == 4 && minor >= 3))
  {
    int blocks = 0;
    glGetIntegerv(GL_MAX_VERTEX_SHADER_STORAGE_BLOCKS, &blocks);
    auto* positions = polydata->GetPointData()->GetArray("blendShapeBindPositions");
    auto* ranges =
      vtkIntArray::SafeDownCast(polydata->GetPointData()->GetArray("blendShapeRanges"));
    auto* offsets = polydata->GetFieldData()->GetArray("blendShapeOffsets");
    auto* weights = polydata->GetFieldData()->GetArray("blendShapeWeights");
    GLint64 maxBlockSize = 0;
    glGetInteger64v(GL_MAX_SHADER_STORAGE_BLOCK_SIZE, &maxBlockSize);
    bool validRanges = false;
    if (ranges && ranges->GetNumberOfComponents() == 2 && ranges->GetNumberOfTuples() && offsets)
    {
      const vtkIdType offsetCount = offsets->GetNumberOfTuples();
      if (this->ValidatedBlendShapeRanges != ranges ||
        ranges->GetMTime() > this->BlendShapeRangesValidationTime ||
        this->ValidatedBlendShapeOffsetCount != offsetCount)
      {
        // Attributes become floats in VTK, so indices above 2^24 can round.
        const vtkIdType limit = std::min<vtkIdType>(offsetCount, 16777216);
        this->BlendShapeRangesValid = true;
        const int* values = ranges->GetPointer(0);
        for (vtkIdType i = 0; i < ranges->GetNumberOfTuples(); ++i)
        {
          if (values[2 * i] < 0 || values[2 * i + 1] < values[2 * i] || values[2 * i + 1] > limit)
          {
            this->BlendShapeRangesValid = false;
            break;
          }
        }
        this->ValidatedBlendShapeRanges = ranges;
        this->ValidatedBlendShapeOffsetCount = offsetCount;
        this->BlendShapeRangesValidationTime.Modified();
      }
      validRanges = this->BlendShapeRangesValid;
    }
    // Leave room for the optional skinning SSBO as well.
    enabled = blocks >= 3 && positions && weights && offsets && validRanges &&
      offsets->GetNumberOfValues() <= maxBlockSize / static_cast<GLint64>(sizeof(float)) &&
      weights->GetNumberOfValues() <= maxBlockSize / static_cast<GLint64>(sizeof(float)) &&
      positions->GetNumberOfComponents() == 3 &&
      positions->GetNumberOfTuples() == polydata->GetNumberOfPoints() &&
      ranges->GetNumberOfTuples() == polydata->GetNumberOfPoints() &&
      this->UpdateMorphingBuffers(polydata);
  }
#endif
  const bool newlyEnabled = enabled && !this->HasSparseMorphing;
  if (enabled != this->HasSparseMorphing)
  {
    this->HasSparseMorphing = enabled;
    this->Modified();
  }
  if (newlyEnabled)
  {
    this->MapDataArrayToVertexAttribute("blendShapeBindPositions", "blendShapeBindPositions",
      vtkDataObject::FIELD_ASSOCIATION_POINTS);
    this->MapDataArrayToVertexAttribute(
      "blendShapeRanges", "blendShapeRanges", vtkDataObject::FIELD_ASSOCIATION_POINTS);
  }
  return enabled;
}

//-----------------------------------------------------------------------------
bool vtkF3DPolyDataMapper::UpdateMorphingBuffers(vtkPolyData* polydata)
{
  auto* offsets =
    vtkFloatArray::SafeDownCast(polydata->GetFieldData()->GetArray("blendShapeOffsets"));
  auto* weights =
    vtkFloatArray::SafeDownCast(polydata->GetFieldData()->GetArray("blendShapeWeights"));
  if (!offsets || offsets->GetNumberOfComponents() != 4 || !offsets->GetNumberOfTuples() ||
    !weights || weights->GetNumberOfComponents() != 1 || !weights->GetNumberOfValues())
  {
    return false;
  }

  if (this->UploadedBlendShapeOffsets != offsets ||
    offsets->GetMTime() > this->BlendShapeOffsetsUploadTime ||
    this->BlendShapeWeights->GetSize() / sizeof(float) !=
      static_cast<size_t>(weights->GetNumberOfValues()))
  {
    for (vtkIdType i = 0; i < offsets->GetNumberOfTuples(); ++i)
    {
      const double index = offsets->GetComponent(i, 3);
      if (!std::isfinite(index) || index < 0. || index >= weights->GetNumberOfValues() ||
        std::floor(index) != index)
      {
        return false;
      }
    }
  }

  auto upload = [](vtkFloatArray* array, vtkOpenGLBufferObject* buffer,
                  vtkWeakPointer<vtkFloatArray>& previous, vtkTimeStamp& uploadTime)
  {
    if (previous != array || array->GetMTime() > uploadTime || !buffer->IsReady())
    {
      if (!buffer->Upload(array->GetPointer(0), static_cast<size_t>(array->GetNumberOfValues()),
            vtkOpenGLBufferObject::ArrayBuffer))
      {
        return false;
      }
      previous = array;
      uploadTime.Modified();
    }
    return true;
  };
  return upload(offsets, this->BlendShapeOffsets, this->UploadedBlendShapeOffsets,
           this->BlendShapeOffsetsUploadTime) &&
    upload(weights, this->BlendShapeWeights, this->UploadedBlendShapeWeights,
      this->BlendShapeWeightsUploadTime) &&
    this->BlendShapeOffsets->BindShaderStorage(1) && this->BlendShapeWeights->BindShaderStorage(2);
}

//-----------------------------------------------------------------------------
void vtkF3DPolyDataMapper::ReleaseGraphicsResources(vtkWindow* window)
{
  if (this->HasSparseMorphing)
  {
    this->HasSparseMorphing = false;
    this->Modified();
  }
  this->BlendShapeOffsets->ReleaseGraphicsResources();
  this->BlendShapeWeights->ReleaseGraphicsResources();
  this->UploadedBlendShapeOffsets = nullptr;
  this->UploadedBlendShapeWeights = nullptr;
  this->Superclass::ReleaseGraphicsResources(window);
}

//-----------------------------------------------------------------------------
void vtkF3DPolyDataMapper::SetUseLinearColorSpace(bool use)
{
  if (this->UseLinearColorSpace != use)
  {
    this->UseLinearColorSpace = use;
    this->ClearColorArrays();
    this->LinearColorTextureScalars = nullptr;
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

  if (this->ColorTextureMap &&
    this->ColorTextureMap->GetPointData()->GetScalars() != this->LinearColorTextureScalars)
  {
    const vtkUnsignedCharArray* source =
      vtkUnsignedCharArray::SafeDownCast(this->ColorTextureMap->GetPointData()->GetScalars());
    const vtkIdType numberOfTuples = source->GetNumberOfTuples();
    this->LinearColorTextureScalars = vtkSmartPointer<vtkUnsignedCharArray>::New();
    this->LinearColorTextureScalars->SetNumberOfComponents(4);
    this->LinearColorTextureScalars->SetNumberOfTuples(numberOfTuples);
    for (vtkIdType i = 0; i < numberOfTuples; ++i)
    {
      for (int component = 0; component < 4; ++component)
      {
        const unsigned char value = source->GetTypedComponent(i, component);
        // Approx. color components converted to linear except alpha channel.
        this->LinearColorTextureScalars->SetTypedComponent(i, component,
          component < 3 ? static_cast<unsigned char>(std::pow(value / 255.0, 2.2) * 255.0) : value);
      }
    }
    this->ColorTextureMap->GetPointData()->SetScalars(this->LinearColorTextureScalars);
  }
  else if (!this->ColorTextureMap)
  {
    this->LinearColorTextureScalars = nullptr;
  }

  return this->Colors;
}

//-----------------------------------------------------------------------------
void vtkF3DPolyDataMapper::ReplaceShaderValues(
  std::map<vtkShader::Type, vtkShader*> shaders, vtkRenderer* ren, vtkActor* actor)
{
  this->Superclass::ReplaceShaderValues(shaders, ren, actor);
  this->HasSSBOSkinning = false;
  auto* morphingVertexShader = shaders[vtkShader::Vertex];
  std::string source = morphingVertexShader->GetSource();
  vtkShaderProgram::Substitute(source, "//F3D::SparsePositionVC::Impl",
    source.find("out vec4 vertexVCVSOutput;") != std::string::npos
      ? "vertexVCVSOutput = MCVCMatrix * posMC;"
      : "");
  morphingVertexShader->SetSource(source);

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

  if (this->HasSparseMorphing)
  {
    this->UpdateMorphingBuffers(this->GetInput());
  }

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

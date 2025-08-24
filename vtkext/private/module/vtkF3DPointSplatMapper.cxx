#include "vtkF3DPointSplatMapper.h"

#include "vtkF3DBitonicSort.h"
#include "vtkF3DComputeDepthCS.h"

#include <vtkCamera.h>
#include <vtkObjectFactory.h>
#include <vtkOpenGLBufferObject.h>
#include <vtkOpenGLIndexBufferObject.h>
#include <vtkOpenGLPointGaussianMapperHelper.h>
#include <vtkOpenGLRenderWindow.h>
#include <vtkOpenGLRenderer.h>
#include <vtkOpenGLShaderCache.h>
#include <vtkOpenGLState.h>
#include <vtkOpenGLVertexBufferObject.h>
#include <vtkOpenGLVertexBufferObjectGroup.h>
#include <vtkPointData.h>
#include <vtkPolyData.h>
#include <vtkShader.h>
#include <vtkShaderProgram.h>
#include <vtkTextureObject.h>
#include <vtkVersion.h>

#if VTK_VERSION_NUMBER >= VTK_VERSION_CHECK(9, 3, 20240914)
#include <vtk_glad.h>
#else
#include <vtk_glew.h>
#endif

#include <sstream>

//----------------------------------------------------------------------------
class vtkF3DSplatMapperHelper : public vtkOpenGLPointGaussianMapperHelper
{
public:
  static vtkF3DSplatMapperHelper* New();
  vtkTypeMacro(vtkF3DSplatMapperHelper, vtkOpenGLPointGaussianMapperHelper);

  vtkF3DSplatMapperHelper(const vtkF3DSplatMapperHelper&) = delete;
  void operator=(const vtkF3DSplatMapperHelper&) = delete;

protected:
  vtkF3DSplatMapperHelper();

  // overridden to create the OpenGL depth buffer
  void BuildBufferObjects(vtkRenderer* ren, vtkActor* act) override;

  // overridden to sort splats
  void RenderPieceDraw(vtkRenderer* ren, vtkActor* act) override;

  // add spherical harmonics
  void ReplaceShaderColor(
    std::map<vtkShader::Type, vtkShader*> shaders, vtkRenderer* ren, vtkActor* actor) override;

  // add spherical texture
  void SetMapperShaderParameters(
    vtkOpenGLHelper& cellBO, vtkRenderer* ren, vtkActor* actor) override;

  // add camera direction
  void SetCameraShaderParameters(
    vtkOpenGLHelper& cellBO, vtkRenderer* ren, vtkActor* actor) override;

private:
  void SortSplats(vtkRenderer* ren);

  vtkNew<vtkShader> DepthComputeShader;
  vtkNew<vtkShaderProgram> DepthProgram;
  vtkNew<vtkOpenGLBufferObject> DepthBuffer;

  vtkNew<vtkF3DBitonicSort> Sorter;

  double DirectionThreshold = 0.999;
  double LastDirection[3] = { 0.0, 0.0, 0.0 };

  int MaxTextureSize = 0;
  vtkNew<vtkTextureObject> SphericalHarmonicsTexture;
  int SphericalHarmonicsDegree = 0;
};

//----------------------------------------------------------------------------
vtkStandardNewMacro(vtkF3DSplatMapperHelper);

//----------------------------------------------------------------------------
vtkF3DSplatMapperHelper::vtkF3DSplatMapperHelper()
{
  this->DepthComputeShader->SetType(vtkShader::Compute);
  this->DepthComputeShader->SetSource(vtkF3DComputeDepthCS);
  this->DepthProgram->SetComputeShader(this->DepthComputeShader);

  this->Sorter->Initialize(512, VTK_FLOAT, VTK_UNSIGNED_INT);
}

//----------------------------------------------------------------------------
void vtkF3DSplatMapperHelper::BuildBufferObjects(vtkRenderer* ren, vtkActor* act)
{
  vtkPolyData* poly = this->CurrentInput;

  if (poly == nullptr)
  {
    return;
  }

  int splatCount = poly->GetPoints()->GetNumberOfPoints();

  vtkOpenGLPointGaussianMapperHelper::BuildBufferObjects(ren, act);

  // allocate a buffer of depths used for sorting splats
  this->DepthBuffer->Allocate(splatCount * sizeof(float), vtkOpenGLBufferObject::ArrayBuffer,
    vtkOpenGLBufferObject::DynamicCopy);

  this->SphericalHarmonicsDegree = 0;

  auto arrayValid = [&](vtkUnsignedCharArray* array)
  {
    if (!array)
    {
      return false;
    }

    bool isValid = array->GetNumberOfComponents() == 3 && array->GetNumberOfTuples() == splatCount;

    if (!isValid)
    {
      vtkWarningMacro("Spherical harmonics array is not valid");
      return false;
    }

    return true;
  };

  vtkUnsignedCharArray* sh1m1 =
    vtkUnsignedCharArray::SafeDownCast(poly->GetPointData()->GetArray("sh1m1"));
  vtkUnsignedCharArray* sh10 =
    vtkUnsignedCharArray::SafeDownCast(poly->GetPointData()->GetArray("sh10"));
  vtkUnsignedCharArray* sh1p1 =
    vtkUnsignedCharArray::SafeDownCast(poly->GetPointData()->GetArray("sh1p1"));

  if (arrayValid(sh1m1) && arrayValid(sh10) && arrayValid(sh1p1))
  {
    // Needs https://gitlab.kitware.com/vtk/vtk/-/merge_requests/12112
#if VTK_VERSION_NUMBER >= VTK_VERSION_CHECK(9, 4, 20250513)

    // we store each spherical harmonics packed in a RGB 8-bits texture.
    // we use the GPU maximum texture size to set the width of the texture.
    // the height will depends on the number of gaussians.
    glGetIntegerv(GL_MAX_TEXTURE_SIZE, &this->MaxTextureSize);

    if (splatCount > this->MaxTextureSize * this->MaxTextureSize)
    {
      vtkWarningMacro("Gaussians count is too high to support spherical harmonics on this GPU");
      return;
    }

    int width = this->MaxTextureSize;
    int height = 1 + (splatCount / this->MaxTextureSize);

    const int sliceSize = 3 * width * height;
    std::vector<unsigned char> packedData(sliceSize * 15, 0);

    std::copy_n(sh1m1->GetPointer(0), sh1m1->GetNumberOfValues(), packedData.data());
    std::copy_n(sh10->GetPointer(0), sh10->GetNumberOfValues(), packedData.data() + sliceSize);
    std::copy_n(
      sh1p1->GetPointer(0), sh1p1->GetNumberOfValues(), packedData.data() + 2 * sliceSize);

    this->SphericalHarmonicsDegree = 1;
    int layers = 3;

    vtkUnsignedCharArray* sh2m2 =
      vtkUnsignedCharArray::SafeDownCast(this->GetInput()->GetPointData()->GetArray("sh2m2"));
    vtkUnsignedCharArray* sh2m1 =
      vtkUnsignedCharArray::SafeDownCast(this->GetInput()->GetPointData()->GetArray("sh2m1"));
    vtkUnsignedCharArray* sh20 =
      vtkUnsignedCharArray::SafeDownCast(this->GetInput()->GetPointData()->GetArray("sh20"));
    vtkUnsignedCharArray* sh2p1 =
      vtkUnsignedCharArray::SafeDownCast(this->GetInput()->GetPointData()->GetArray("sh2p1"));
    vtkUnsignedCharArray* sh2p2 =
      vtkUnsignedCharArray::SafeDownCast(this->GetInput()->GetPointData()->GetArray("sh2p2"));

    if (arrayValid(sh2m2) && arrayValid(sh2m1) && arrayValid(sh20) && arrayValid(sh2p1) &&
      arrayValid(sh2p2))
    {
      std::copy_n(
        sh2m2->GetPointer(0), sh2m2->GetNumberOfValues(), packedData.data() + 3 * sliceSize);
      std::copy_n(
        sh2m1->GetPointer(0), sh2m1->GetNumberOfValues(), packedData.data() + 4 * sliceSize);
      std::copy_n(
        sh20->GetPointer(0), sh20->GetNumberOfValues(), packedData.data() + 5 * sliceSize);
      std::copy_n(
        sh2p1->GetPointer(0), sh2p1->GetNumberOfValues(), packedData.data() + 6 * sliceSize);
      std::copy_n(
        sh2p2->GetPointer(0), sh2p2->GetNumberOfValues(), packedData.data() + 7 * sliceSize);

      this->SphericalHarmonicsDegree = 2;
      layers = 8;

      vtkUnsignedCharArray* sh3m3 =
        vtkUnsignedCharArray::SafeDownCast(this->GetInput()->GetPointData()->GetArray("sh3m3"));
      vtkUnsignedCharArray* sh3m2 =
        vtkUnsignedCharArray::SafeDownCast(this->GetInput()->GetPointData()->GetArray("sh3m2"));
      vtkUnsignedCharArray* sh3m1 =
        vtkUnsignedCharArray::SafeDownCast(this->GetInput()->GetPointData()->GetArray("sh3m1"));
      vtkUnsignedCharArray* sh30 =
        vtkUnsignedCharArray::SafeDownCast(this->GetInput()->GetPointData()->GetArray("sh30"));
      vtkUnsignedCharArray* sh3p1 =
        vtkUnsignedCharArray::SafeDownCast(this->GetInput()->GetPointData()->GetArray("sh3p1"));
      vtkUnsignedCharArray* sh3p2 =
        vtkUnsignedCharArray::SafeDownCast(this->GetInput()->GetPointData()->GetArray("sh3p2"));
      vtkUnsignedCharArray* sh3p3 =
        vtkUnsignedCharArray::SafeDownCast(this->GetInput()->GetPointData()->GetArray("sh3p3"));

      if (arrayValid(sh3m3) && arrayValid(sh3m2) && arrayValid(sh3m1) && arrayValid(sh30) &&
        arrayValid(sh3p1) && arrayValid(sh3p2) && arrayValid(sh3p3))
      {
        std::copy_n(
          sh3m3->GetPointer(0), sh3m3->GetNumberOfValues(), packedData.data() + 8 * sliceSize);
        std::copy_n(
          sh3m2->GetPointer(0), sh3m2->GetNumberOfValues(), packedData.data() + 9 * sliceSize);
        std::copy_n(
          sh3m1->GetPointer(0), sh3m1->GetNumberOfValues(), packedData.data() + 10 * sliceSize);
        std::copy_n(
          sh30->GetPointer(0), sh30->GetNumberOfValues(), packedData.data() + 11 * sliceSize);
        std::copy_n(
          sh3p1->GetPointer(0), sh3p1->GetNumberOfValues(), packedData.data() + 12 * sliceSize);
        std::copy_n(
          sh3p2->GetPointer(0), sh3p2->GetNumberOfValues(), packedData.data() + 13 * sliceSize);
        std::copy_n(
          sh3p3->GetPointer(0), sh3p3->GetNumberOfValues(), packedData.data() + 14 * sliceSize);

        this->SphericalHarmonicsDegree = 3;
        layers = 15;
      }
    }

    this->SphericalHarmonicsTexture->SetContext(
      static_cast<vtkOpenGLRenderWindow*>(ren->GetRenderWindow()));
    this->SphericalHarmonicsTexture->Create2DArrayFromRaw(
      width, height, 3, VTK_UNSIGNED_CHAR, layers, packedData.data());
#else
    vtkWarningMacro("VTK < 9.5.0 does not support gaussian spherical harmonics");
#endif
  }
}

//------------------------------------------------------------------------------
void vtkF3DSplatMapperHelper::SetMapperShaderParameters(
  vtkOpenGLHelper& cellBO, vtkRenderer* ren, vtkActor* actor)
{
  if (this->SphericalHarmonicsDegree > 0)
  {
    this->SphericalHarmonicsTexture->Activate();
    cellBO.Program->SetUniformi(
      "sphericalHarmonics", this->SphericalHarmonicsTexture->GetTextureUnit());
  }

  this->Superclass::SetMapperShaderParameters(cellBO, ren, actor);
}

//------------------------------------------------------------------------------
void vtkF3DSplatMapperHelper::SetCameraShaderParameters(
  vtkOpenGLHelper& cellBO, vtkRenderer* ren, vtkActor* actor)
{
  if (this->SphericalHarmonicsDegree > 0)
  {
    cellBO.Program->SetUniform3f(
      "cameraDirection", ren->GetActiveCamera()->GetDirectionOfProjection());
  }

  this->Superclass::SetCameraShaderParameters(cellBO, ren, actor);
}

//----------------------------------------------------------------------------
void vtkF3DSplatMapperHelper::SortSplats(vtkRenderer* ren)
{
  int numVerts = this->VBOs->GetNumberOfTuples("vertexMC");

  if (numVerts)
  {
    const double* focalPoint = ren->GetActiveCamera()->GetFocalPoint();
    const double* origin = ren->GetActiveCamera()->GetPosition();
    double direction[3];

    for (int i = 0; i < 3; ++i)
    {
      // the orientation is reverted to sort splats back to front
      direction[i] = origin[i] - focalPoint[i];
    }

    vtkMath::Normalize(direction);

    // sort the splats only if the camera direction has changed
    if (vtkMath::Dot(this->LastDirection, direction) < this->DirectionThreshold)
    {
      vtkOpenGLShaderCache* shaderCache =
        vtkOpenGLRenderWindow::SafeDownCast(ren->GetRenderWindow())->GetShaderCache();

      // compute next power of two
      unsigned int numVertsExt = vtkMath::NearestPowerOfTwo(numVerts);

      // depth computation
      shaderCache->ReadyShaderProgram(this->DepthProgram);

      this->LastDirection[0] = direction[0];
      this->LastDirection[1] = direction[1];
      this->LastDirection[2] = direction[2];

      this->DepthProgram->SetUniform3f("viewDirection", direction);
      this->DepthProgram->SetUniformi("count", numVerts);
      this->VBOs->GetVBO("vertexMC")->BindShaderStorage(0);
      this->Primitives[PrimitivePoints].IBO->BindShaderStorage(1);
      this->DepthBuffer->BindShaderStorage(2);

      glDispatchCompute(numVertsExt / 32, 1, 1);
      glMemoryBarrier(GL_SHADER_STORAGE_BARRIER_BIT);

      // sort
      this->Sorter->Run(vtkOpenGLRenderWindow::SafeDownCast(ren->GetRenderWindow()), numVerts,
        this->DepthBuffer, this->Primitives[PrimitivePoints].IBO);
    }
  }
}

//----------------------------------------------------------------------------
void vtkF3DSplatMapperHelper::RenderPieceDraw(vtkRenderer* ren, vtkActor* actor)
{
  if (vtkShader::IsComputeShaderSupported() && actor->GetForceTranslucent())
  {
    this->SortSplats(ren);
  }

  vtkOpenGLPointGaussianMapperHelper::RenderPieceDraw(ren, actor);
}

//----------------------------------------------------------------------------
void vtkF3DSplatMapperHelper::ReplaceShaderColor(
  std::map<vtkShader::Type, vtkShader*> shaders, vtkRenderer* ren, vtkActor* actor)
{
  if (this->VBOs->GetNumberOfComponents("scalarColor") != 0 && !this->DrawingVertices)
  {
    std::string VSSource = shaders[vtkShader::Vertex]->GetSource();

    vtkShaderProgram::Substitute(VSSource, "//VTK::Color::Dec",
      "//VTK::Color::Dec\n\n"
      "uniform sampler2DArray sphericalHarmonics;\n"
      "uniform vec3 cameraDirection;\n"
      "vec3 decode(ivec3 texelIndex)\n"
      "{\n"
      "  vec3 texel = texelFetch(sphericalHarmonics, texelIndex, 0).rgb;\n"
      "  return texel * 2.0 - 1.0;\n"
      "}\n\n",
      false);

    std::stringstream shStr;
    shStr << "//VTK::Color::Impl\n";

    if (this->SphericalHarmonicsDegree >= 1)
    {
      shStr << "  vec3 sh1 = vec3(0);\n";
      shStr << "  ivec2 texelIndex = ivec2(gl_VertexID % " << this->MaxTextureSize
            << ", gl_VertexID / " << this->MaxTextureSize << ");\n";
      shStr << "  sh1 -= 0.48860251 * decode(ivec3(texelIndex, 0)) * cameraDirection.y;\n";
      shStr << "  sh1 += 0.48860251 * decode(ivec3(texelIndex, 1)) * cameraDirection.z;\n";
      shStr << "  sh1 -= 0.48860251 * decode(ivec3(texelIndex, 2)) * cameraDirection.x;\n";
      shStr << "  vertexColorVSOutput.rgb += sh1;\n";
    }

    if (this->SphericalHarmonicsDegree >= 2)
    {
      shStr << "  float xx = cameraDirection.x * cameraDirection.x;\n";
      shStr << "  float yy = cameraDirection.y * cameraDirection.y;\n";
      shStr << "  float zz = cameraDirection.z * cameraDirection.z;\n";
      shStr << "  float xy = cameraDirection.x * cameraDirection.y;\n";
      shStr << "  float yz = cameraDirection.y * cameraDirection.z;\n";
      shStr << "  float xz = cameraDirection.x * cameraDirection.z;\n";
      shStr << "  vec3 sh2 = vec3(0);\n";
      shStr << "  sh2 += 1.092548430 * decode(ivec3(texelIndex, 3)) * xy;\n";
      shStr << "  sh2 -= 1.09254843 * decode(ivec3(texelIndex, 4)) * yz;\n";
      shStr << "  sh2 += 0.315391565 * decode(ivec3(texelIndex, 5)) * (3.0 * zz - 1.0);\n";
      shStr << "  sh2 -= 1.09254843 * decode(ivec3(texelIndex, 6)) * xz;\n";
      shStr << "  sh2 += 0.546274215 * decode(ivec3(texelIndex, 7)) * (xx - yy);\n";
      shStr << "  vertexColorVSOutput.rgb += sh2;\n";
    }

    if (this->SphericalHarmonicsDegree >= 3)
    {
      shStr << "  vec3 sh3 = vec3(0);\n";
      shStr
        << "  sh3 -= 0.59004358 * decode(ivec3(texelIndex, 8)) * cameraDirection.y * (3.0 * xx - "
           "yy);\n";
      shStr << "  sh3 += 2.890611442 * decode(ivec3(texelIndex, 9)) * xy * cameraDirection.z;\n";
      shStr
        << "  sh3 -= 0.45704579 * decode(ivec3(texelIndex, 10)) * cameraDirection.y * (4.0 * zz "
           "- xx - yy);\n";
      shStr
        << "  sh3 += 0.373176332 * decode(ivec3(texelIndex, 11)) * cameraDirection.z * (2.0 * zz "
           "- 3.0 * xx - 3.0 * yy);\n";
      shStr
        << "  sh3 -= 0.45704579 * decode(ivec3(texelIndex, 12)) * cameraDirection.x * (4.0 * zz "
           "- xx - yy);\n";
      shStr << "  sh3 += 1.445305721 * decode(ivec3(texelIndex, 13)) * cameraDirection.z * (xx - "
               "yy);\n";
      shStr
        << "  sh3 -= 0.59004358 * decode(ivec3(texelIndex, 14)) * cameraDirection.x * (xx - 3.0 "
           "* yy);\n";
      shStr << "  vertexColorVSOutput.rgb += sh3;\n";
    }

    // With spherical harmonics, we can have very small negative values resulting in blending
    // errors. The color needs to be clamped to positive values to avoid this.
    shStr << "  vertexColorVSOutput.rgb = max(vertexColorVSOutput.rgb, vec3(0.));\n";

    vtkShaderProgram::Substitute(VSSource, "//VTK::Color::Impl", shStr.str(), false);

    shaders[vtkShader::Vertex]->SetSource(VSSource);
  }

  this->Superclass::ReplaceShaderColor(shaders, ren, actor);
}

//----------------------------------------------------------------------------
vtkOpenGLPointGaussianMapperHelper* vtkF3DPointSplatMapper::CreateHelper()
{
  auto helper = vtkF3DSplatMapperHelper::New();
  helper->Owner = this;
  return helper;
}

//----------------------------------------------------------------------------
vtkStandardNewMacro(vtkF3DPointSplatMapper);

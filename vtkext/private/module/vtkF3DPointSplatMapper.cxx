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
#include <vtkPolyData.h>
#include <vtkShader.h>
#include <vtkShaderProgram.h>
#include <vtkVersion.h>

#if VTK_VERSION_NUMBER >= VTK_VERSION_CHECK(9, 3, 20240914)
#include <vtk_glad.h>
#else
#include <vtk_glew.h>
#endif

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

private:
  void SortSplats(vtkRenderer* ren);

  vtkNew<vtkShader> DepthComputeShader;
  vtkNew<vtkShaderProgram> DepthProgram;
  vtkNew<vtkOpenGLBufferObject> DepthBuffer;

  vtkNew<vtkF3DBitonicSort> Sorter;

  double DirectionThreshold = 0.999;
  double LastDirection[3] = { 0.0, 0.0, 0.0 };
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

  this->DepthBuffer->Allocate(splatCount * sizeof(float), vtkOpenGLBufferObject::ArrayBuffer,
    vtkOpenGLBufferObject::DynamicCopy);
}

//----------------------------------------------------------------------------
void vtkF3DSplatMapperHelper::SortSplats(vtkRenderer* ren)
{
  int numVerts = this->VBOs->GetNumberOfTuples("vertexMC");

  if (numVerts)
  {
    double* focalPoint = ren->GetActiveCamera()->GetFocalPoint();
    double* origin = ren->GetActiveCamera()->GetPosition();
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
vtkOpenGLPointGaussianMapperHelper* vtkF3DPointSplatMapper::CreateHelper()
{
  auto helper = vtkF3DSplatMapperHelper::New();
  helper->Owner = this;
  return helper;
}

//----------------------------------------------------------------------------
vtkStandardNewMacro(vtkF3DPointSplatMapper);

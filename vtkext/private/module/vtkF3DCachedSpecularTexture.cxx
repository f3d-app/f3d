#include "vtkF3DCachedSpecularTexture.h"

#include <vtkImageData.h>
#include <vtkMultiBlockDataSet.h>
#include <vtkObjectFactory.h>
#include <vtkOpenGLRenderWindow.h>
#include <vtkRenderer.h>
#include <vtkTextureObject.h>
#include <vtkVersion.h>
#include <vtkXMLMultiBlockDataReader.h>

#if VTK_VERSION_NUMBER >= VTK_VERSION_CHECK(9, 3, 20240914)
#include <vtk_glad.h>
#else
#include <vtk_glew.h>
#endif

vtkStandardNewMacro(vtkF3DCachedSpecularTexture);

//------------------------------------------------------------------------------
void vtkF3DCachedSpecularTexture::PrintSelf(ostream& os, vtkIndent indent)
{
  this->Superclass::PrintSelf(os, indent);
  os << indent << "FileName: " << this->FileName << "\n";
}

//------------------------------------------------------------------------------
void vtkF3DCachedSpecularTexture::Load(vtkRenderer* ren)
{
  if (!this->UseCache)
  {
    return this->Superclass::Load(ren);
  }

  if (this->GetMTime() > this->LoadTime.GetMTime())
  {
    vtkOpenGLRenderWindow* renWin = vtkOpenGLRenderWindow::SafeDownCast(ren->GetRenderWindow());

    if (this->TextureObject == nullptr)
    {
      this->TextureObject = vtkTextureObject::New();
    }

    this->TextureObject->SetContext(renWin);
    this->TextureObject->SetFormat(GL_RGB);
    this->TextureObject->SetInternalFormat(GL_RGB32F);
    this->TextureObject->SetDataType(GL_FLOAT);
    this->TextureObject->SetWrapS(vtkTextureObject::ClampToEdge);
    this->TextureObject->SetWrapT(vtkTextureObject::ClampToEdge);
    this->TextureObject->SetWrapR(vtkTextureObject::ClampToEdge);
    this->TextureObject->SetMinificationFilter(vtkTextureObject::LinearMipmapLinear);
    this->TextureObject->SetMagnificationFilter(vtkTextureObject::Linear);
    this->TextureObject->SetGenerateMipmap(true);

    this->RenderWindow = renWin;

    vtkNew<vtkXMLMultiBlockDataReader> reader;
    reader->SetFileName(this->FileName.c_str());
    reader->Update();

    vtkMultiBlockDataSet* mb = vtkMultiBlockDataSet::SafeDownCast(reader->GetOutput());

    unsigned int nbLevels = mb->GetNumberOfBlocks();

    this->TextureObject->SetMaxLevel(static_cast<int>(nbLevels) - 1);

    vtkImageData* firstImg = vtkImageData::SafeDownCast(mb->GetBlock(0));

    void* data[6];
    for (int i = 0; i < 6; i++)
    {
      data[i] = firstImg->GetScalarPointer(0, 0, i);
    }

    const int* firstDims = firstImg->GetDimensions();
    if (firstDims[0] != firstDims[1])
    {
      vtkWarningMacro("Specular cache has unexpected dimensions");
    }
    this->PrefilterSize = firstDims[0];
    this->TextureObject->CreateCubeFromRaw(
      this->PrefilterSize, this->PrefilterSize, 3, VTK_FLOAT, data);

    // the mip levels are manually uploaded because there is no abstraction in VTK
    for (unsigned int i = 1; i < nbLevels; i++)
    {
      vtkImageData* img = vtkImageData::SafeDownCast(mb->GetBlock(i));
      int* dims = img->GetDimensions();

      for (int j = 0; j < 6; j++)
      {
        glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + j, static_cast<GLint>(i),
          this->TextureObject->GetInternalFormat(VTK_FLOAT, 3, false), static_cast<GLint>(dims[0]),
          static_cast<GLint>(dims[1]), 0, this->TextureObject->GetFormat(VTK_FLOAT, 3, false),
          this->TextureObject->GetDataType(VTK_FLOAT), img->GetScalarPointer(0, 0, j));
      }
    }

    this->LoadTime.Modified();
  }

  this->TextureObject->Activate();
}

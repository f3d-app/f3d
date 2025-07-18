#include "vtkF3DCachedLUTTexture.h"

#include <vtkImageData.h>
#include <vtkObjectFactory.h>
#include <vtkOpenGLRenderWindow.h>
#include <vtkRenderer.h>
#include <vtkTextureObject.h>
#include <vtkVersion.h>
#include <vtkXMLImageDataReader.h>

#if VTK_VERSION_NUMBER >= VTK_VERSION_CHECK(9, 3, 20240914)
#include <vtk_glad.h>
#else
#include <vtk_glew.h>
#endif

vtkStandardNewMacro(vtkF3DCachedLUTTexture);

//------------------------------------------------------------------------------
void vtkF3DCachedLUTTexture::PrintSelf(ostream& os, vtkIndent indent)
{
  this->Superclass::PrintSelf(os, indent);
  os << indent << "FileName: " << this->FileName << "\n";
}

//------------------------------------------------------------------------------
void vtkF3DCachedLUTTexture::Load(vtkRenderer* ren)
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
    this->TextureObject->SetFormat(GL_RG);
#ifdef GL_ES_VERSION_3_0
    this->TextureObject->SetInternalFormat(GL_RG8);
    this->TextureObject->SetDataType(GL_UNSIGNED_BYTE);
#else
    this->TextureObject->SetInternalFormat(GL_RG16);
    this->TextureObject->SetDataType(GL_UNSIGNED_SHORT);
#endif
    this->TextureObject->SetWrapS(vtkTextureObject::ClampToEdge);
    this->TextureObject->SetWrapT(vtkTextureObject::ClampToEdge);
    this->TextureObject->SetMinificationFilter(vtkTextureObject::Linear);
    this->TextureObject->SetMagnificationFilter(vtkTextureObject::Linear);

    vtkNew<vtkXMLImageDataReader> reader;
    reader->SetFileName(this->FileName.c_str());
    reader->Update();

    vtkImageData* img = reader->GetOutput();
    int dims[3];
    img->GetDimensions(dims);
    if (dims[0] != dims[1])
    {
      vtkWarningMacro("LUT cache has unexpected dimensions");
    }
    this->LUTSize = dims[0];

#ifdef GL_ES_VERSION_3_0
    this->TextureObject->Create2DFromRaw(
      this->LUTSize, this->LUTSize, 2, VTK_UNSIGNED_CHAR, img->GetScalarPointer());
#else
    this->TextureObject->Create2DFromRaw(
      this->LUTSize, this->LUTSize, 2, VTK_UNSIGNED_SHORT, img->GetScalarPointer());
#endif

    this->RenderWindow = renWin;
    this->LoadTime.Modified();
  }

  this->TextureObject->Activate();
}

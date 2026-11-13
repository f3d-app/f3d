#include "vtkF3DPSDReader.h"

#include <cstring>
#include <vtkFileResourceStream.h>
#include <vtkImageData.h>
#include <vtkMemoryResourceStream.h>
#include <vtkNew.h>
#include <vtkObjectFactory.h>
#include <vtkPointData.h>
#include <vtkUnsignedCharArray.h>
#include <vtkVersion.h>

#include <algorithm>
#include <string_view>

#define STB_IMAGE_IMPLEMENTATION
#define STBI_ONLY_PSD
#define STBI_NO_STDIO
#include "stb_image.h"

vtkStandardNewMacro(vtkF3DPSDReader);

//------------------------------------------------------------------------------
vtkF3DPSDReader::vtkF3DPSDReader() = default;

//------------------------------------------------------------------------------
vtkF3DPSDReader::~vtkF3DPSDReader() = default;

//------------------------------------------------------------------------------
void vtkF3DPSDReader::ExecuteInformation()
{
  // XXX: Needed because of VTK initialize file pattern in the constructor for some reasons
  delete[] this->FilePattern;
  this->FilePattern = nullptr;

  // Setup filename to read the header
  this->ComputeInternalFileName(this->DataExtent[4]);
  if ((this->InternalFileName == nullptr || this->InternalFileName[0] == '\0'))
  {
#if VTK_VERSION_NUMBER >= VTK_VERSION_CHECK(9, 5, 20251016)
    if (!this->GetStream())
#else
    if (!this->GetMemoryBuffer())
#endif
    {
      return;
    }
  }

  vtkResourceStream* stream;
  vtkNew<vtkFileResourceStream> fileStream;

#if VTK_VERSION_NUMBER >= VTK_VERSION_CHECK(9, 5, 20251016)
  if (this->GetStream())
  {
    stream = this->GetStream();
  }
#else
  vtkNew<vtkMemoryResourceStream> memStream;
  if (this->GetMemoryBuffer())
  {
    memStream->SetBuffer(this->GetMemoryBuffer(), this->GetMemoryBufferLength());
    stream = memStream;
  }
#endif
  else
  {
    fileStream->Open(this->InternalFileName);
    stream = fileStream;
  }

  stream->Seek(0, vtkResourceStream::SeekDirection::End);
  this->BufferData.resize(stream->Tell());

  stream->Seek(0, vtkResourceStream::SeekDirection::Begin);
  stream->Read(this->BufferData.data(), this->BufferData.size());

  int width, height, comp;
  if (!stbi_info_from_memory(
        this->BufferData.data(), this->BufferData.size(), &width, &height, &comp))
  {
    vtkErrorMacro(<< "Could not get PSD infos");
    return;
  }

  this->DataExtent[0] = 0;
  this->DataExtent[1] = width - 1;
  this->DataExtent[2] = 0;
  this->DataExtent[3] = height - 1;

  this->SetNumberOfScalarComponents(4);
  this->SetDataScalarTypeToUnsignedChar();

  this->vtkImageReader::ExecuteInformation();
}

//------------------------------------------------------------------------------
int vtkF3DPSDReader::CanReadFile(const char* fname)
{
  vtkNew<vtkFileResourceStream> fileStream;
  if (!fileStream->Open(fname))
  {
    vtkErrorMacro(<< "Could not open file " << fname);
    return 0;
  }
  return this->CanReadFile(fileStream);
}

//------------------------------------------------------------------------------
int vtkF3DPSDReader::CanReadFile(vtkResourceStream* stream)
{
  if (!stream)
  {
    return 0;
  }

  stream->Seek(0, vtkResourceStream::SeekDirection::Begin);

  char header[4];
  if (stream->Read(header, 4) != 4)
  {
    return 0;
  }

  const std::string_view sv(header, 4);
  return sv == "8BPS";
}

//------------------------------------------------------------------------------
void vtkF3DPSDReader::ExecuteDataWithInformation(vtkDataObject* output, vtkInformation* outInfo)
{
  vtkImageData* data = this->AllocateOutputData(output, outInfo);

  vtkUnsignedCharArray* scalars =
    vtkUnsignedCharArray::SafeDownCast(data->GetPointData()->GetScalars());
  if (!scalars)
  {
    vtkErrorMacro(<< "Could not find expected scalar array");
    return;
  }

  scalars->SetName("Pixels");

  stbi_set_flip_vertically_on_load(1);
  int width, height, comp;
  unsigned char* pixels = stbi_load_from_memory(static_cast<uint8_t*>(this->BufferData.data()),
    this->BufferData.size(), &width, &height, &comp, 4);
  if (!pixels)
  {
    vtkErrorMacro(<< "Could not decode PSD: " << stbi_failure_reason());
    return;
  }
  std::copy_n(pixels, scalars->GetNumberOfValues(), scalars->GetPointer(0));
  stbi_image_free(pixels);
}

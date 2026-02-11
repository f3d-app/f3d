#include "vtkF3DWebPReader.h"

#include <vtkFileResourceStream.h>
#include <vtkImageData.h>
#include <vtkMemoryResourceStream.h>
#include <vtkNew.h>
#include <vtkObjectFactory.h>
#include <vtkPointData.h>
#include <vtkUnsignedCharArray.h>
#include <vtkVersion.h>
#include <vtksys/FStream.hxx>

#include "webp/decode.h"

#include <algorithm>
#include <string_view>

vtkStandardNewMacro(vtkF3DWebPReader);

//------------------------------------------------------------------------------
vtkF3DWebPReader::vtkF3DWebPReader() = default;

//------------------------------------------------------------------------------
vtkF3DWebPReader::~vtkF3DWebPReader() = default;

//------------------------------------------------------------------------------
void vtkF3DWebPReader::ExecuteInformation()
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

  int width = -1, height = -1;

  if (!WebPGetInfo(this->BufferData.data(), this->BufferData.size(), &width, &height))
  {
    vtkErrorMacro(<< "Could get webp infos");
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
int vtkF3DWebPReader::CanReadFile(const char* fname)
{
#if VTK_VERSION_NUMBER >= VTK_VERSION_CHECK(9, 6, 20260106)
  vtkNew<vtkFileResourceStream> fileStream;
  if (!fileStream->Open(fname))
  {
    vtkErrorMacro(<< "Could not open file " << fname);
    return 0;
  }
  return this->CanReadFile(fileStream);
#else
  vtksys::ifstream ifs(fname, vtksys::ifstream::in | vtksys::ifstream::binary);

  if (ifs.fail())
  {
    vtkErrorMacro(<< "Could not open file " << fname);
    return 0;
  }

  char header[12];
  ifs.read(header, 12);
  if (ifs.gcount() != 12)
  {
    return 0;
  }

  std::string_view sv(header, 12);
  return sv.substr(0, 4) == "RIFF" && sv.substr(8, 4) == "WEBP";
#endif
}

#if VTK_VERSION_NUMBER >= VTK_VERSION_CHECK(9, 6, 20260106)
//------------------------------------------------------------------------------
int vtkF3DWebPReader::CanReadFile(vtkResourceStream* stream)
{
  if (!stream)
  {
    return 0;
  }

  stream->Seek(0, vtkResourceStream::SeekDirection::Begin);

  char header[12];
  if (stream->Read(header, 12) != 12)
  {
    return 0;
  }

  std::string_view sv(header, 12);
  return sv.substr(0, 4) == "RIFF" && sv.substr(8, 4) == "WEBP";
}
#endif

//------------------------------------------------------------------------------
void vtkF3DWebPReader::ExecuteDataWithInformation(vtkDataObject* output, vtkInformation* outInfo)
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

  uint8_t* pixels = WebPDecodeRGBA(
    static_cast<uint8_t*>(this->BufferData.data()), this->BufferData.size(), nullptr, nullptr);
  std::copy_n(pixels, scalars->GetSize(), scalars->GetPointer(0));

  WebPFree(pixels);
}

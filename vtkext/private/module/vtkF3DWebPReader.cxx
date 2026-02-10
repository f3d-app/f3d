#include "vtkF3DWebPReader.h"

#include <vtkFileResourceStream.h>
#include <vtkImageData.h>
#include <vtkMemoryResourceStream.h>
#include <vtkNew.h>
#include <vtkObjectFactory.h>
#include <vtkPointData.h>
#include <vtkResourceParser.h>
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
  // get the magic number by reading in a file
  vtksys::ifstream ifs(fname, vtksys::ifstream::in);

  if (ifs.fail())
  {
    vtkErrorMacro(<< "Could not open file " << fname);
    return 0;
  }

  // The file must begin with magic number RIFF
  if ((ifs.get() != 'R') || (ifs.get() != 'I') || (ifs.get() != 'F') || (ifs.get() != 'F'))
  {
    ifs.close();
    return 0;
  }

  // Skip 4 bytes (file size field) to get to position 8
  ifs.ignore(4);

  // Check for WEBP signature at bytes 8-11
  if ((ifs.get() != 'W') || (ifs.get() != 'E') || (ifs.get() != 'B') || (ifs.get() != 'P'))
  {
    ifs.close();
    return 0;
  }

  ifs.close();
  return 1;
}

#if VTK_VERSION_NUMBER >= VTK_VERSION_CHECK(9, 6, 20260116)
//------------------------------------------------------------------------------
int vtkF3DWebPReader::CanReadFile(vtkResourceStream* stream)
{
  if (!stream)
  {
    return 0;
  }

  stream->Seek(0, vtkResourceStream::SeekDirection::Begin);

  vtkNew<vtkResourceParser> parser;
  parser->SetStream(stream);

  std::string header;
  parser->ReadLine(header, 12);
  if (header.size() != 12)
  {
    return 0;
  }

  std::string_view sv(header);
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

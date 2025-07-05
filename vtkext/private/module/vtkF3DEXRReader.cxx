#include "vtkF3DEXRReader.h"

#include "vtkFloatArray.h"
#include "vtkImageData.h"
#include "vtkNew.h"
#include "vtkObjectFactory.h"
#include "vtkPointData.h"
#include "vtksys/FStream.hxx"

#include <ImfArray.h>
#include <ImfIO.h>
#include <ImfRgbaFile.h>

#include <algorithm>
#include <sstream>
#include <thread>

/**
 * Class implementing a memory stream for OpenEXR
 */
class MemStream : public Imf::IStream
{
public:
  MemStream(const char* name, const void* buff, vtkIdType bufferLen)
    : Imf::IStream(name)
    , Buffer(static_cast<const char*>(buff))
    , BuffLen(static_cast<size_t>(bufferLen))
  {
  }

  bool read(char content[], int size) override
  {
    if (this->Pos + size <= this->BuffLen)
    {
      std::copy_n(this->Buffer + this->Pos, size, content);
      this->Pos += size;
      return true;
    }
    return false;
  }

  /**
   * returns the current reading position, in bytes, from the beginning of the file.
   * The next read() call will begin reading at the indicated position
   */
  uint64_t tellg() override
  {
    return this->Pos;
  }

  /**
   * sets the current reading position to pos bytes from the beginning of the "file"
   */
  void seekg(uint64_t newPos) override
  {
    this->Pos = newPos;
  }

private:
  const char* Buffer;
  size_t BuffLen;
  uint64_t Pos = 0;
};

vtkStandardNewMacro(vtkF3DEXRReader);

//------------------------------------------------------------------------------
vtkF3DEXRReader::vtkF3DEXRReader() = default;

//------------------------------------------------------------------------------
vtkF3DEXRReader::~vtkF3DEXRReader() = default;

//------------------------------------------------------------------------------
void vtkF3DEXRReader::PrintSelf(ostream& os, vtkIndent indent)
{
  this->Superclass::PrintSelf(os, indent);
}

//------------------------------------------------------------------------------
void vtkF3DEXRReader::ExecuteInformation()
{
  // XXX: Needed because of VTK initialize file pattern in the constructor for some reasons
  delete[] this->FilePattern;
  this->FilePattern = nullptr;

  // Setup filename to read the header
  this->ComputeInternalFileName(this->DataExtent[4]);
  if ((this->InternalFileName == nullptr || this->InternalFileName[0] == '\0') &&
    !this->MemoryBuffer)
  {
    return;
  }

  auto checkChannels = [&](Imf::RgbaInputFile& file)
  {
    Imath::Box2i dw = file.dataWindow();
    this->DataExtent[0] = dw.min.x;
    this->DataExtent[1] = dw.max.x;
    this->DataExtent[2] = dw.min.y;
    this->DataExtent[3] = dw.max.y;

    Imf::RgbaChannels channels = file.channels();
    if (channels != Imf::RgbaChannels::WRITE_RGBA && channels != Imf::RgbaChannels::WRITE_RGB)
    {
      throw std::runtime_error("only RGB and RGBA channels are supported");
    }
  };

  try
  {
    if (this->MemoryBuffer)
    {
      MemStream memoryStream("EXRmemoryStream", this->MemoryBuffer, this->MemoryBufferLength);
      Imf::RgbaInputFile file = Imf::RgbaInputFile(memoryStream);
      checkChannels(file);
    }
    else
    {
      Imf::RgbaInputFile file(this->InternalFileName);
      checkChannels(file);
    }
  }
  catch (const std::exception& e)
  {
    vtkErrorMacro("Error reading EXR file: " << e.what());
    return;
  }

  this->SetNumberOfScalarComponents(3);
  this->SetDataScalarTypeToFloat();

  this->vtkImageReader::ExecuteInformation();
}

//------------------------------------------------------------------------------
int vtkF3DEXRReader::CanReadFile(const char* fname)
{
  // get the magic number by reading in a file
  vtksys::ifstream ifs(fname, vtksys::ifstream::in);

  if (ifs.fail())
  {
    vtkErrorMacro(<< "Could not open file " << fname);
    return 0;
  }

  // The file must begin with magic number 76 2F 31 01
  if ((ifs.get() != 0x76) || (ifs.get() != 0x2F) || (ifs.get() != 0x31) || (ifs.get() != 0x01))
  {
    ifs.close();
    return 0;
  }

  ifs.close();
  return 1;
}

//------------------------------------------------------------------------------
void vtkF3DEXRReader::ExecuteDataWithInformation(vtkDataObject* output, vtkInformation* outInfo)
{
  vtkImageData* data = this->AllocateOutputData(output, outInfo);

  if (this->UpdateExtentIsEmpty(outInfo, output))
  {
    return;
  }

  vtkFloatArray* scalars = vtkFloatArray::SafeDownCast(data->GetPointData()->GetScalars());
  if (!scalars)
  {
    vtkErrorMacro(<< "Could not find expected scalar array");
    return;
  }

  scalars->SetName("Pixels");
  float* dataPtr = scalars->GetPointer(0);

  auto readContent = [&](Imf::RgbaInputFile& file)
  {
    Imf::Array2D<Imf::Rgba> pixels(this->GetHeight(), this->GetWidth());

    file.setFrameBuffer(&pixels[0][0], 1, this->GetWidth());
    file.readPixels(this->DataExtent[2], this->DataExtent[3]);

    for (int y = this->GetHeight() - 1; y >= 0; y--)
    {
      for (int x = 0; x < this->GetWidth(); x++)
      {
        const Imf::Rgba& p = pixels[y][x];
        dataPtr[0] = std::clamp(static_cast<float>(p.r), 0.f, 10000.f);
        dataPtr[1] = std::clamp(static_cast<float>(p.g), 0.f, 10000.f);
        dataPtr[2] = std::clamp(static_cast<float>(p.b), 0.f, 10000.f);
        dataPtr += 3;
      }
    }
  };

  try
  {
    Imf::setGlobalThreadCount(std::thread::hardware_concurrency());

    if (this->MemoryBuffer)
    {
      MemStream memoryStream("EXRmemoryStream", this->MemoryBuffer, this->MemoryBufferLength);
      Imf::RgbaInputFile file = Imf::RgbaInputFile(memoryStream);
      readContent(file);
    }
    else
    {
      Imf::RgbaInputFile file(this->InternalFileName);
      readContent(file);
    }
  }
  catch (const std::exception& e)
  {
    vtkErrorMacro("Error reading EXR file: " << e.what());
  }
}

//------------------------------------------------------------------------------
int vtkF3DEXRReader::GetWidth() const
{
  return this->DataExtent[1] - this->DataExtent[0] + 1;
}

//------------------------------------------------------------------------------
int vtkF3DEXRReader::GetHeight() const
{
  return this->DataExtent[3] - this->DataExtent[2] + 1;
}

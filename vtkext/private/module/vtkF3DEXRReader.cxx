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

#include <cstring>
#include <sstream>
#include <thread>

/**
 * Class to treat file contents in memory like it were still in a file.
 */
class MemStream : public Imf::IStream
{
public:
  MemStream(const char* name, const void* buff, vtkIdType bufferLen)
    : Imf::IStream(name)
    , buffer(static_cast<const char*>(buff))
    , bufflen(static_cast<size_t>(bufferLen))
    , pos(0)
  {
  }

  bool read(char c[], int n) override
  {
    if (pos + n <= bufflen)
    {
      memcpy(c, buffer + pos, n);
      pos += n;
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
    return pos;
  }

  /**
   * sets the current reading position to pos bytes from the beginning of the "file"
   */
  void seekg(uint64_t new_pos) override
  {
    pos = new_pos;
  }

private:
  const char* buffer;
  size_t bufflen;
  uint64_t pos{ 0 };
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
  if (this->InternalFileName == nullptr || this->InternalFileName[0] == '\0')
  {
    // If we have no file then maybe we have the file in memory
    if (!this->MemoryBuffer)
      return;
  }

  auto execute = [&](Imf::RgbaInputFile& file)
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
      execute(file);
    }
    else
    {
      Imf::RgbaInputFile file(this->InternalFileName);
      execute(file);
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

  auto execute = [&](Imf::RgbaInputFile& file)
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
    assert(this->InternalFileName);
    Imf::setGlobalThreadCount(std::thread::hardware_concurrency());

    if (this->MemoryBuffer)
    {
      MemStream memoryStream("EXRmemoryStream", this->MemoryBuffer, this->MemoryBufferLength);
      Imf::RgbaInputFile file = Imf::RgbaInputFile(memoryStream);
      execute(file);
    }
    else
    {
      Imf::RgbaInputFile file(this->InternalFileName);
      execute(file);
    }
  }
  catch (const std::exception& e)
  {
    vtkErrorMacro("Error reading EXR file: " << e.what());
    return;
  }
}

//------------------------------------------------------------------------------
/**
 * Read from memory instead of file
 */
void vtkF3DEXRReader::SetMemoryBuffer(const void* buff)
{
  MemoryBuffer = buff;
}

//------------------------------------------------------------------------------
/**
 * Specify the in memory image buffer length.
 */
void vtkF3DEXRReader::SetMemoryBufferLength(vtkIdType bufferLen)
{
  MemoryBufferLength = bufferLen;
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

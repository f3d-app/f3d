#include "vtkF3DSPZReader.h"

#include <vtkFileResourceStream.h>
#include <vtkFloatArray.h>
#include <vtkInformation.h>
#include <vtkInformationVector.h>
#include <vtkObjectFactory.h>
#include <vtkPointData.h>
#include <vtkPoints.h>
#include <vtkStreamingDemandDrivenPipeline.h>
#include <vtk_zlib.h>

#include <algorithm>

namespace
{
bool UncompressGzip(
  const std::vector<unsigned char>& compressed, std::vector<unsigned char>& uncompressed)
{
  std::vector<uint8_t> buffer(8192);
  z_stream stream = {};
  stream.next_in = const_cast<Bytef*>(compressed.data());
  stream.avail_in = compressed.size();
  if (inflateInit2(&stream, 16 | MAX_WBITS) != Z_OK)
  {
    return false;
  }
  bool success = false;
  while (true)
  {
    stream.next_out = buffer.data();
    stream.avail_out = buffer.size();
    int32_t res = inflate(&stream, Z_NO_FLUSH);
    if (res != Z_OK && res != Z_STREAM_END)
    {
      break;
    }
    uncompressed.insert(
      uncompressed.end(), buffer.data(), buffer.data() + buffer.size() - stream.avail_out);
    if (res == Z_STREAM_END)
    {
      success = true;
      break;
    }
  }
  inflateEnd(&stream);
  return success;
}
}

vtkStandardNewMacro(vtkF3DSPZReader);

//----------------------------------------------------------------------------
int vtkF3DSPZReader::RequestData(
  vtkInformation*, vtkInformationVector**, vtkInformationVector* outputVector)
{
  vtkPolyData* output = vtkPolyData::GetData(outputVector);

  vtkSmartPointer<vtkResourceStream> stream;

  if (this->Stream)
  {
    stream = this->Stream;
    if (this->Stream->SupportSeek())
    {
      this->Stream->Seek(0, vtkResourceStream::SeekDirection::Begin);
    }
  }
  else
  {
    vtkNew<vtkFileResourceStream> fileStream;
    if (!this->FileName || !fileStream->Open(this->FileName))
    {
      vtkErrorMacro(<< "Failed to open file: "
                    << (this->FileName ? this->FileName : "No file name set"));
      return 0;
    }

    stream = fileStream;
  }

  size_t compressedLength = 0;

  if (stream->SupportSeek())
  {
    stream->Seek(0, vtkResourceStream::SeekDirection::End);
    compressedLength = stream->Tell();

    if (compressedLength < 4)
    {
      vtkErrorMacro("Invalid stream content");
      return 0;
    }

    stream->Seek(0, vtkResourceStream::SeekDirection::Begin);
  }

  std::vector<unsigned char> compressed(compressedLength);

  stream->Read(compressed.data(), compressedLength);

  // get the buffer size in order to pre-allocate
  uint32_t uncompressedLength = static_cast<uint32_t>(compressed[compressedLength - 4]) |
    (static_cast<uint32_t>(compressed[compressedLength - 3]) << 8) |
    (static_cast<uint32_t>(compressed[compressedLength - 2]) << 16) |
    (static_cast<uint32_t>(compressed[compressedLength - 1]) << 24);

  std::vector<unsigned char> uncompressed;
  uncompressed.reserve(uncompressedLength);

  if (!::UncompressGzip(compressed, uncompressed))
  {
    vtkErrorMacro("Uncompress failed");
    return 0;
  }

  struct PackedGaussiansHeader
  {
    uint32_t magic;
    uint32_t version;
    uint32_t numPoints;
    uint8_t shDegree;
    uint8_t fractionalBits;
    uint8_t flags;
    uint8_t reserved;
  };

  PackedGaussiansHeader* header = reinterpret_cast<PackedGaussiansHeader*>(uncompressed.data());

  if (header->magic != 0x5053474e)
  {
    vtkErrorMacro("Magic number doesn't match");
    return 0;
  }

  if (header->version != 2)
  {
    vtkErrorMacro("Unsupported version");
    return 0;
  }

  uint32_t nbSplats = header->numPoints;

  vtkNew<vtkFloatArray> positionArray;
  positionArray->SetNumberOfComponents(3);
  positionArray->SetNumberOfTuples(nbSplats);
  positionArray->SetName("position");

  unsigned char* position = uncompressed.data() + 16;

  float positionScale = 1.0 / (1 << header->fractionalBits);

  for (size_t splatIndex = 0; splatIndex < nbSplats; splatIndex++)
  {
    for (size_t c = 0; c < 3; c++)
    {
      int32_t fixed32 = position[0];
      fixed32 |= position[1] << 8;
      fixed32 |= position[2] << 16;
      fixed32 |= (fixed32 & 0x800000) ? 0xff000000 : 0; // sign extension

      position += 3;

      positionArray->SetTypedComponent(splatIndex, c, static_cast<float>(fixed32) * positionScale);
    }
  }

  vtkNew<vtkUnsignedCharArray> colorArray;
  colorArray->SetNumberOfComponents(4);
  colorArray->SetNumberOfTuples(nbSplats);
  colorArray->SetName("color");

  unsigned char* alpha = uncompressed.data() + 16 + 9 * nbSplats;
  unsigned char* color = uncompressed.data() + 16 + (9 + 1) * nbSplats;

  for (size_t splatIndex = 0; splatIndex < nbSplats; splatIndex++)
  {
    for (size_t c = 0; c < 3; c++)
    {
      constexpr float colorScale = 0.15f;
      constexpr float SH_C0 = 0.28209479177387814;

      float fcolor = ((color[c] / 255.0f) - 0.5f) / colorScale;
      fcolor = std::clamp(fcolor * SH_C0 + 0.5, 0.0, 1.0);

      colorArray->SetTypedComponent(splatIndex, c, static_cast<unsigned char>(fcolor * 255.0));
    }

    colorArray->SetTypedComponent(splatIndex, 3, *alpha);

    color += 3;
    alpha++;
  }

  vtkNew<vtkFloatArray> scaleArray;
  scaleArray->SetNumberOfComponents(3);
  scaleArray->SetNumberOfTuples(nbSplats);
  scaleArray->SetName("scale");

  unsigned char* scale = uncompressed.data() + 16 + (9 + 4) * nbSplats;

  for (size_t splatIndex = 0; splatIndex < nbSplats; splatIndex++)
  {
    for (size_t c = 0; c < 3; c++)
    {
      scaleArray->SetTypedComponent(splatIndex, c, std::exp((scale[c] / 16.0f - 10.0f)));
    }

    scale += 3;
  }

  unsigned char* rotation = uncompressed.data() + 16 + (9 + 4 + 3) * nbSplats;

  vtkNew<vtkFloatArray> rotationArray;
  rotationArray->SetNumberOfComponents(4);
  rotationArray->SetNumberOfTuples(nbSplats);
  rotationArray->SetName("rotation");

  for (size_t splatIndex = 0; splatIndex < nbSplats; splatIndex++)
  {
    float xyz[3] = { static_cast<float>(rotation[0]) / 127.5f - 1.f,
      static_cast<float>(rotation[1]) / 127.5f - 1.f,
      static_cast<float>(rotation[2]) / 127.5f - 1.f };

    float w = std::sqrt(std::max(0.0f, 1.0f - vtkMath::Dot(xyz, xyz)));

    rotationArray->SetTypedComponent(splatIndex, 0, w);
    rotationArray->SetTypedComponent(splatIndex, 1, xyz[0]);
    rotationArray->SetTypedComponent(splatIndex, 2, xyz[1]);
    rotationArray->SetTypedComponent(splatIndex, 3, xyz[2]);

    rotation += 3;
  }

  vtkNew<vtkPoints> points;
  points->SetDataTypeToFloat();
  points->SetData(positionArray);
  output->SetPoints(points);

  output->GetPointData()->SetScalars(colorArray);
  output->GetPointData()->AddArray(scaleArray);
  output->GetPointData()->AddArray(rotationArray);

  return 1;
}

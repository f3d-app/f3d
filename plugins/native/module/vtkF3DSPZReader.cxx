#include "vtkF3DSPZReader.h"

#include <vtkFileResourceStream.h>
#include <vtkFloatArray.h>
#include <vtkInformation.h>
#include <vtkInformationVector.h>
#include <vtkObjectFactory.h>
#include <vtkPointData.h>
#include <vtkPoints.h>
#include <vtkStreamingDemandDrivenPipeline.h>
#include <vtkVersion.h>
#include <vtk_zlib.h>

#include <algorithm>

namespace
{
//----------------------------------------------------------------------------
bool UncompressGzip(
  const std::vector<unsigned char>& compressed, std::vector<unsigned char>& uncompressed)
{
  std::vector<uint8_t> buffer(8192);

  z_stream stream = {};
  stream.next_in = const_cast<Bytef*>(compressed.data());
  stream.avail_in = static_cast<unsigned int>(compressed.size());

  inflateInit2(&stream, 16 | MAX_WBITS);

  int32_t res = Z_OK;

  while (res != Z_STREAM_END)
  {
    stream.next_out = buffer.data();
    stream.avail_out = static_cast<unsigned int>(buffer.size());

    res = inflate(&stream, Z_NO_FLUSH);
    if (!(res == Z_OK || res == Z_STREAM_END))
    {
      return false;
    }

    uncompressed.insert(
      uncompressed.end(), buffer.data(), buffer.data() + buffer.size() - stream.avail_out);
  }
  inflateEnd(&stream);
  return true;
}

//----------------------------------------------------------------------------
struct Header
{
  uint32_t magic;
  uint32_t version;
  uint32_t numPoints;
  uint8_t shDegree;
  uint8_t fractionalBits;
  uint8_t flags;
  uint8_t reserved;
};

//----------------------------------------------------------------------------
struct PackedCoordinate
{
  uint8_t packed[3]; // 24-bits

  float decode(float scale) const
  {
    int32_t fixed32 = packed[0];
    fixed32 |= packed[1] << 8;
    fixed32 |= packed[2] << 16;
    fixed32 |= (fixed32 & 0x800000) ? 0xff000000 : 0; // sign
    return static_cast<float>(fixed32) * scale;
  }
};

//----------------------------------------------------------------------------
struct ColorChannel
{
  uint8_t channel;

  uint8_t decode() const
  {
    constexpr float colorScale = 0.15f;
    constexpr float SH_C0 = 0.28209479177387814;

    float fcolor = ((channel / 255.0f) - 0.5f) / colorScale;
    fcolor = std::clamp(fcolor * SH_C0 + 0.5, 0.0, 1.0);

    return static_cast<uint8_t>(fcolor * 255.0);
  }
};

//----------------------------------------------------------------------------
struct LogScale
{
  uint8_t scale;

  float decode() const
  {
    return std::exp((scale / 16.0f - 10.0f));
  }
};

//----------------------------------------------------------------------------
struct PackedRotationV2
{
  uint8_t packed[3]; // 24-bits

  std::array<float, 4> decode() const
  {
    float xyz[3] = { static_cast<float>(packed[0]) / 127.5f - 1.f,
      static_cast<float>(packed[1]) / 127.5f - 1.f, static_cast<float>(packed[2]) / 127.5f - 1.f };

    float w = std::sqrt(std::max(0.0f, 1.0f - vtkMath::Dot(xyz, xyz)));

    return { w, xyz[0], xyz[1], xyz[2] };
  }
};

//----------------------------------------------------------------------------
struct PackedRotationV3
{
  uint8_t packed[4]; // 32-bits

  std::array<float, 4> decode() const
  {
    float rotation[4];
    constexpr float sqrt12 = 0.707106781186547524401f; // 1/sqrt(2)
    constexpr uint32_t compMask = (1u << 9u) - 1u;

    uint32_t comp = packed[0] + (packed[1] << 8) + (packed[2] << 16) + (packed[3] << 24);

    const int largestCompIndex = comp >> 30;
    float sumSquares = 0;
    for (int i = 3; i >= 0; --i)
    {
      if (i != largestCompIndex)
      {
        float mag = comp & compMask;
        uint32_t isNegative = (comp >> 9u) & 0x1u;
        comp = comp >> 10u;
        rotation[i] = sqrt12 * (mag / float(compMask));
        if (isNegative == 1)
        {
          rotation[i] = -rotation[i];
        }
        sumSquares += rotation[i] * rotation[i];
      }
    }
    rotation[largestCompIndex] = std::sqrt(1.0f - sumSquares);

    return { rotation[0], rotation[1], rotation[2], rotation[3] };
  }
};

//----------------------------------------------------------------------------
template<int Degree>
struct SphericalHarmonics
{
  uint8_t packed[3 * Degree * (Degree + 2)];

  template<int M, int L>
  std::array<uint8_t, 3> decode() const
  {
    static_assert(L > 0 && L <= Degree);
    static_assert(M <= L && M >= -L);

    constexpr int offset = 3 * ((M + L) + L * L - 1);

    std::array<uint8_t, 3> sh;
    for (int i = 0; i < 3; i++)
    {
      sh[i] = packed[offset + i];
    }
    return sh;
  }
};

//----------------------------------------------------------------------------
template<int Degree>
void AddSphericalHarmonics(int nbSplats, unsigned char* buffer, vtkPointData* pointData)
{
  SphericalHarmonics<Degree>* begin =
    reinterpret_cast<SphericalHarmonics<Degree>*>(buffer + 16 + (9 + 4 + 3 + 3) * nbSplats);

  auto getSuffix = [](int m) -> std::string
  {
    if (m == 0)
    {
      return "0";
    }
    if (m > 0)
    {
      return std::string("p") + std::to_string(m);
    }
    return std::string("m") + std::to_string(-m);
  };

  vtkNew<vtkUnsignedCharArray> sh1Array[3];

  for (int i = 0; i < 3; i++)
  {
    sh1Array[i]->SetNumberOfComponents(3);
    sh1Array[i]->SetNumberOfTuples(nbSplats);
    sh1Array[i]->SetName((std::string("sh1") + getSuffix(i - 1)).data());
    pointData->AddArray(sh1Array[i]);
  }

  SphericalHarmonics<Degree>* sh = begin;

  for (int splatIndex = 0; splatIndex < nbSplats; splatIndex++, sh++)
  {
    sh1Array[0]->SetTypedTuple(splatIndex, sh->template decode<-1, 1>().data());
    sh1Array[1]->SetTypedTuple(splatIndex, sh->template decode<0, 1>().data());
    sh1Array[2]->SetTypedTuple(splatIndex, sh->template decode<1, 1>().data());
  }

  if constexpr (Degree >= 2)
  {
    vtkNew<vtkUnsignedCharArray> sh2Array[5];

    for (int i = 0; i < 5; i++)
    {
      sh2Array[i]->SetNumberOfComponents(3);
      sh2Array[i]->SetNumberOfTuples(nbSplats);
      sh2Array[i]->SetName((std::string("sh2") + getSuffix(i - 2)).data());
      pointData->AddArray(sh2Array[i]);
    }

    sh = begin;

    for (int splatIndex = 0; splatIndex < nbSplats; splatIndex++, sh++)
    {
      sh2Array[0]->SetTypedTuple(splatIndex, sh->template decode<-2, 2>().data());
      sh2Array[1]->SetTypedTuple(splatIndex, sh->template decode<-1, 2>().data());
      sh2Array[2]->SetTypedTuple(splatIndex, sh->template decode<0, 2>().data());
      sh2Array[3]->SetTypedTuple(splatIndex, sh->template decode<1, 2>().data());
      sh2Array[4]->SetTypedTuple(splatIndex, sh->template decode<2, 2>().data());
    }
  }

  if constexpr (Degree >= 3)
  {
    vtkNew<vtkUnsignedCharArray> sh3Array[7];

    for (int i = 0; i < 7; i++)
    {
      sh3Array[i]->SetNumberOfComponents(3);
      sh3Array[i]->SetNumberOfTuples(nbSplats);
      sh3Array[i]->SetName((std::string("sh3") + getSuffix(i - 3)).data());
      pointData->AddArray(sh3Array[i]);
    }

    sh = begin;

    for (int splatIndex = 0; splatIndex < nbSplats; splatIndex++, sh++)
    {
      sh3Array[0]->SetTypedTuple(splatIndex, sh->template decode<-3, 3>().data());
      sh3Array[1]->SetTypedTuple(splatIndex, sh->template decode<-2, 3>().data());
      sh3Array[2]->SetTypedTuple(splatIndex, sh->template decode<-1, 3>().data());
      sh3Array[3]->SetTypedTuple(splatIndex, sh->template decode<0, 3>().data());
      sh3Array[4]->SetTypedTuple(splatIndex, sh->template decode<1, 3>().data());
      sh3Array[5]->SetTypedTuple(splatIndex, sh->template decode<2, 3>().data());
      sh3Array[6]->SetTypedTuple(splatIndex, sh->template decode<3, 3>().data());
    }
  }
}
}

//----------------------------------------------------------------------------
vtkStandardNewMacro(vtkF3DSPZReader);

//----------------------------------------------------------------------------
int vtkF3DSPZReader::RequestData(
  vtkInformation*, vtkInformationVector**, vtkInformationVector* outputVector)
{
  vtkPolyData* output = vtkPolyData::GetData(outputVector);

  vtkNew<vtkPoints> points;
  points->SetDataTypeToFloat();
  output->SetPoints(points);

  vtkSmartPointer<vtkResourceStream> stream;

#if VTK_VERSION_NUMBER > VTK_VERSION_CHECK(9, 4, 20250501)
  if (this->Stream)
  {
    stream = this->Stream;
    assert(this->Stream->SupportSeek());
  }
  else
#endif
  {
    vtkNew<vtkFileResourceStream> fileStream;
    fileStream->Open(this->FileName);
    stream = fileStream;
  }

  stream->Seek(0, vtkResourceStream::SeekDirection::End);
  size_t compressedLength = stream->Tell();

  stream->Seek(0, vtkResourceStream::SeekDirection::Begin);

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
    vtkErrorMacro("Invalid GZIP file");
    return 0;
  }

  const Header* header = reinterpret_cast<Header*>(uncompressed.data());

  if (header->magic != 0x5053474e)
  {
    vtkErrorMacro("Incompatible SPZ header");
    return 0;
  }

  if (header->version < 2 || header->version > 3)
  {
    vtkErrorMacro("Incompatible SPZ version. Only 2 and 3 are supported");
    return 0;
  }

  uint32_t nbSplats = header->numPoints;

  vtkNew<vtkFloatArray> positionArray;
  positionArray->SetNumberOfComponents(3);
  positionArray->SetNumberOfTuples(nbSplats);
  positionArray->SetName("position");

  // position is stored just after the 16-bytes header
  PackedCoordinate* position = reinterpret_cast<PackedCoordinate*>(uncompressed.data() + 16);

  float positionScale = 1.0 / (1 << header->fractionalBits);

  for (vtkIdType splatIndex = 0; splatIndex < static_cast<vtkIdType>(nbSplats); splatIndex++)
  {
    for (int c = 0; c < 3; c++)
    {
      positionArray->SetTypedComponent(splatIndex, c, (position++)->decode(positionScale));
    }
  }

  points->SetData(positionArray);

  vtkNew<vtkUnsignedCharArray> colorArray;
  colorArray->SetNumberOfComponents(4);
  colorArray->SetNumberOfTuples(nbSplats);
  colorArray->SetName("color");

  // alpha is stored just after the 16-bytes header and the positions
  unsigned char* alpha = uncompressed.data() + 16 + 9 * nbSplats;

  // color is stored just after the 16-bytes header and alphas
  ColorChannel* color =
    reinterpret_cast<ColorChannel*>(uncompressed.data() + 16 + (9 + 1) * nbSplats);

  for (vtkIdType splatIndex = 0; splatIndex < static_cast<vtkIdType>(nbSplats); splatIndex++)
  {
    for (int c = 0; c < 3; c++)
    {
      colorArray->SetTypedComponent(splatIndex, c, (color++)->decode());
    }

    colorArray->SetTypedComponent(splatIndex, 3, *alpha++);
  }

  output->GetPointData()->SetScalars(colorArray);

  vtkNew<vtkFloatArray> scaleArray;
  scaleArray->SetNumberOfComponents(3);
  scaleArray->SetNumberOfTuples(nbSplats);
  scaleArray->SetName("scale");

  // scale is stored just after the 16-bytes header, positions, colors and alphas
  LogScale* scale = reinterpret_cast<LogScale*>(uncompressed.data() + 16 + (9 + 4) * nbSplats);

  for (vtkIdType splatIndex = 0; splatIndex < static_cast<vtkIdType>(nbSplats); splatIndex++)
  {
    for (int c = 0; c < 3; c++)
    {
      scaleArray->SetTypedComponent(splatIndex, c, (scale++)->decode());
    }
  }

  output->GetPointData()->AddArray(scaleArray);

  vtkNew<vtkFloatArray> rotationArray;
  rotationArray->SetNumberOfComponents(4);
  rotationArray->SetNumberOfTuples(nbSplats);
  rotationArray->SetName("rotation");

  // rotation is stored just after the 16-bytes header, positions, colors, alphas and scales
  int rotationShift = 16 + (9 + 4 + 3) * nbSplats;

  if (header->version == 2)
  {
    PackedRotationV2* rotation =
      reinterpret_cast<PackedRotationV2*>(uncompressed.data() + rotationShift);
    for (vtkIdType splatIndex = 0; splatIndex < static_cast<vtkIdType>(nbSplats); splatIndex++)
    {
      rotationArray->SetTypedTuple(splatIndex, (rotation++)->decode().data());
    }
  }
  else if (header->version == 3)
  {
    PackedRotationV3* rotation =
      reinterpret_cast<PackedRotationV3*>(uncompressed.data() + rotationShift);
    for (vtkIdType splatIndex = 0; splatIndex < static_cast<vtkIdType>(nbSplats); splatIndex++)
    {
      rotationArray->SetTypedTuple(splatIndex, (rotation++)->decode().data());
    }
  }

  output->GetPointData()->AddArray(rotationArray);

  switch (header->shDegree)
  {
    case 1:
      AddSphericalHarmonics<1>(nbSplats, uncompressed.data(), output->GetPointData());
      break;
    case 2:
      AddSphericalHarmonics<2>(nbSplats, uncompressed.data(), output->GetPointData());
      break;
    case 3:
      AddSphericalHarmonics<3>(nbSplats, uncompressed.data(), output->GetPointData());
      break;
    default: // nothing to add
      break;
  }

  return 1;
}

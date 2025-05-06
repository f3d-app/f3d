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
struct PackedRotation
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
template<int Degree>
struct SphericalHarmonics
{
  uint8_t packed[3 * Degree * (Degree + 2)];

  template<int M, int L>
  std::array<float, 3> decode() const
  {
    static_assert(L > 0 && L <= Degree);
    static_assert(M <= L && M >= -L);

    constexpr int offset = 3 * ((M + L) + L * L - 1);

    std::array<float, 3> sh;
    for (int i = 0; i < 3; i++)
    {
      sh[i] = constant<M,L>() * unquantize(packed[offset + i]);
    }
    return sh;
  }

private:
  static float unquantize(unsigned char c)
  {
    return c * (2.f / 255.f) - 1.f;
  }

  template<int M, int L>
  static constexpr float constant()
  {
    constexpr float SH_C1[3] = { -0.48860251, 0.48860251, -0.48860251 };
    constexpr float SH_C2[5] = { 1.092548430, -1.09254843, 0.315391565, -1.09254843, 0.546274215 };
    constexpr float SH_C3[7] = { -0.59004358, 2.890611442, -0.45704579, 0.373176332, -0.45704579,
      1.445305721, -0.59004358 };

    if constexpr (L == 1)
    {
      return SH_C1[M + 1];
    }
    if constexpr (L == 2)
    {
      return SH_C2[M + 2];
    }
    if constexpr (L == 3)
    {
      return SH_C3[M + 3];
    }
    return 0.f;
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

  vtkNew<vtkFloatArray> sh1Array[3];

  for (int i = 0; i < 3; i++)
  {
    sh1Array[i]->SetNumberOfComponents(3);
    sh1Array[i]->SetNumberOfTuples(nbSplats);
    sh1Array[i]->SetName((std::string("sh1") + getSuffix(i - 1)).data());
    pointData->AddArray(sh1Array[i]);
  }

  SphericalHarmonics<Degree>* sh = begin;

  for (size_t splatIndex = 0; splatIndex < nbSplats; splatIndex++, sh++)
  {
    sh1Array[0]->SetTypedTuple(splatIndex, sh->template decode<-1, 1>().data());
    sh1Array[1]->SetTypedTuple(splatIndex, sh->template decode<0, 1>().data());
    sh1Array[2]->SetTypedTuple(splatIndex, sh->template decode<1, 1>().data());
  }

  if constexpr (Degree >= 2)
  {
    vtkNew<vtkFloatArray> sh2Array[5];

    for (int i = 0; i < 5; i++)
    {
      sh2Array[i]->SetNumberOfComponents(3);
      sh2Array[i]->SetNumberOfTuples(nbSplats);
      sh2Array[i]->SetName((std::string("sh2") + getSuffix(i - 2)).data());
      pointData->AddArray(sh2Array[i]);
    }

    sh = begin;

    for (size_t splatIndex = 0; splatIndex < nbSplats; splatIndex++, sh++)
    {
      sh2Array[0]->SetTypedTuple(splatIndex, sh->template decode<-2, 2>().data());
      sh2Array[1]->SetTypedTuple(splatIndex, sh->template decode<-1, 2>().data());
      sh2Array[2]->SetTypedTuple(splatIndex, sh->template decode<0, 2>().data());
      sh2Array[3]->SetTypedTuple(splatIndex, sh->template decode<1, 2>().data());
      sh2Array[4]->SetTypedTuple(splatIndex, sh->template decode<2, 2>().data());
    }
  }

  /*
  if constexpr (Degree >= 3)
  {
    vtkNew<vtkFloatArray> sh3Array;
    sh3Array->SetNumberOfComponents(21);
    sh3Array->SetNumberOfTuples(nbSplats);
    sh3Array->SetName("sh3");

    sh = begin;

    for (size_t splatIndex = 0; splatIndex < nbSplats; splatIndex++, sh++)
    {
      sh3Array->SetTypedTuple(splatIndex, (sh++)->template decode<3>().data());
    }

    // pointData->AddArray(sh3Array);
  }
    */
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
    if (!this->FileName || !fileStream->Open(this->FileName))
    {
      vtkErrorMacro(<< "Failed to open file: "
                    << (this->FileName ? this->FileName : "No file name set"));
      return 0;
    }

    stream = fileStream;
  }

  stream->Seek(0, vtkResourceStream::SeekDirection::End);
  size_t compressedLength = stream->Tell();

  if (compressedLength < 4)
  {
    vtkErrorMacro("Invalid stream content");
    return 0;
  }

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
    vtkErrorMacro("Uncompress failed");
    return 0;
  }

  Header* header = reinterpret_cast<Header*>(uncompressed.data());

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

  // position is stored just after the 16-bytes header
  PackedCoordinate* position = reinterpret_cast<PackedCoordinate*>(uncompressed.data() + 16);

  float positionScale = 1.0 / (1 << header->fractionalBits);

  for (size_t splatIndex = 0; splatIndex < nbSplats; splatIndex++)
  {
    for (size_t c = 0; c < 3; c++)
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

  for (size_t splatIndex = 0; splatIndex < nbSplats; splatIndex++)
  {
    for (size_t c = 0; c < 3; c++)
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

  for (size_t splatIndex = 0; splatIndex < nbSplats; splatIndex++)
  {
    for (size_t c = 0; c < 3; c++)
    {
      scaleArray->SetTypedComponent(splatIndex, c, (scale++)->decode());
    }
  }

  output->GetPointData()->AddArray(scaleArray);

  // rotation is stored just after the 16-bytes header, positions, colors, alphas and scales
  PackedRotation* rotation =
    reinterpret_cast<PackedRotation*>(uncompressed.data() + 16 + (9 + 4 + 3) * nbSplats);

  vtkNew<vtkFloatArray> rotationArray;
  rotationArray->SetNumberOfComponents(4);
  rotationArray->SetNumberOfTuples(nbSplats);
  rotationArray->SetName("rotation");

  for (size_t splatIndex = 0; splatIndex < nbSplats; splatIndex++)
  {
    rotationArray->SetTypedTuple(splatIndex, (rotation++)->decode().data());
  }

  output->GetPointData()->AddArray(rotationArray);

  AddSphericalHarmonics<1>(nbSplats, uncompressed.data(), output->GetPointData());

  switch (header->shDegree)
  {
    case 0: // nothing to add
      break;
    case 1:
      AddSphericalHarmonics<1>(nbSplats, uncompressed.data(), output->GetPointData());
      break;
    case 2:
      AddSphericalHarmonics<2>(nbSplats, uncompressed.data(), output->GetPointData());
      break;
    case 3:
      AddSphericalHarmonics<3>(nbSplats, uncompressed.data(), output->GetPointData());
      break;
    default:
      vtkWarningMacro("Invalid spherical harmonics degree, ignoring");
      break;
  }

  return 1;
}

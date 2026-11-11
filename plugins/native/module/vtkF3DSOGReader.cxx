#include "vtkF3DSOGReader.h"

#include "vtkF3DArchiveReader.h"
#include "vtkF3DWebPReader.h"

#include <vtkFileResourceStream.h>
#include <vtkFloatArray.h>
#include <vtkImageData.h>
#include <vtkInformation.h>
#include <vtkInformationVector.h>
#include <vtkMemoryResourceStream.h>
#include <vtkNew.h>
#include <vtkObjectFactory.h>
#include <vtkPointData.h>
#include <vtkPoints.h>
#include <vtkPolyData.h>
#include <vtkResourceStream.h>
#include <vtkSmartPointer.h>
#include <vtkUnsignedCharArray.h>
#include <vtkVersion.h>

#include <nlohmann/json.hpp>

#include <algorithm>
#include <array>
#include <cmath>
#include <cstdint>
#include <string>
#include <vector>

namespace
{
// SetBuffer does not copy.
struct Image
{
  std::vector<char> Bytes;
  vtkSmartPointer<vtkImageData> Data;
};

//----------------------------------------------------------------------------
int Width(const Image& image)
{
  return image.Data->GetDimensions()[0];
}

//----------------------------------------------------------------------------
int Height(const Image& image)
{
  return image.Data->GetDimensions()[1];
}

//----------------------------------------------------------------------------
bool ReadFiles(const nlohmann::json& node, std::size_t count, std::vector<std::string>& names)
{
  if (!node.is_object() || !node.contains("files") || !node["files"].is_array() ||
    node["files"].size() < count)
  {
    return false;
  }

  names.resize(count);
  for (std::size_t i = 0; i < count; ++i)
  {
    if (!node["files"][i].is_string())
    {
      return false;
    }
    names[i] = node["files"][i].get<std::string>();
  }
  return true;
}

//----------------------------------------------------------------------------
bool ReadRange(const nlohmann::json& node, const char* key, double range[3])
{
  if (!node.is_object() || !node.contains(key) || !node[key].is_array() || node[key].size() != 3)
  {
    return false;
  }

  for (int axis = 0; axis < 3; ++axis)
  {
    if (!node[key][axis].is_number())
    {
      return false;
    }
    range[axis] = node[key][axis].get<double>();
  }
  return true;
}

//----------------------------------------------------------------------------
bool ReadCodebook(const nlohmann::json& node, std::array<double, 256>& codebook)
{
  if (!node.is_object() || !node.contains("codebook") || !node["codebook"].is_array() ||
    node["codebook"].size() != codebook.size())
  {
    return false;
  }

  for (std::size_t i = 0; i < codebook.size(); ++i)
  {
    if (!node["codebook"][i].is_number())
    {
      return false;
    }
    codebook[i] = node["codebook"][i].get<double>();
  }
  return true;
}

//----------------------------------------------------------------------------
bool ReadImage(vtkF3DArchiveReader* archive, const std::string& name, Image& image)
{
  if (!archive->Extract(name, image.Bytes) || image.Bytes.empty())
  {
    return false;
  }

  vtkNew<vtkF3DWebPReader> webp;
#if VTK_VERSION_NUMBER >= VTK_VERSION_CHECK(9, 5, 20251016)
  vtkNew<vtkMemoryResourceStream> stream;
  stream->SetBuffer(image.Bytes.data(), image.Bytes.size());
  webp->SetStream(stream);
#else
  webp->SetMemoryBuffer(image.Bytes.data());
  webp->SetMemoryBufferLength(static_cast<vtkIdType>(image.Bytes.size()));
#endif
  webp->Update();

  image.Data = webp->GetOutput();
  if (!image.Data)
  {
    return false;
  }

  const int width = ::Width(image);
  const int height = ::Height(image);
  if (width <= 0 || height <= 0)
  {
    return false;
  }

  vtkUnsignedCharArray* scalars =
    vtkUnsignedCharArray::SafeDownCast(image.Data->GetPointData()->GetScalars());
  const vtkIdType valueCount = static_cast<vtkIdType>(width) * static_cast<vtkIdType>(height) * 4;
  return scalars && scalars->GetNumberOfComponents() == 4 &&
    scalars->GetNumberOfValues() >= valueCount;
}

//----------------------------------------------------------------------------
const uint8_t* Pixel(const Image& image, vtkIdType index)
{
  vtkUnsignedCharArray* scalars =
    vtkUnsignedCharArray::SafeDownCast(image.Data->GetPointData()->GetScalars());
  return scalars->GetPointer(static_cast<vtkIdType>(static_cast<std::size_t>(index) * 4));
}

//----------------------------------------------------------------------------
float DecodePosition(uint8_t low, uint8_t high, double minimum, double maximum)
{
  const unsigned int quantized = (static_cast<unsigned int>(high) << 8) | low;
  const double normalized =
    minimum + (maximum - minimum) * (static_cast<double>(quantized) / 65535.0);
  const double magnitude = std::exp(std::abs(normalized)) - 1.0;
  return static_cast<float>(std::copysign(magnitude, normalized));
}

//----------------------------------------------------------------------------
float DecodeScale(double coefficient)
{
  return static_cast<float>(std::exp(coefficient));
}

//----------------------------------------------------------------------------
unsigned char DecodeColor(double coefficient)
{
  constexpr double shC0 = 0.28209479177387814;
  const double color = std::clamp(0.5 + coefficient * shC0, 0.0, 1.0);
  return static_cast<unsigned char>(color * 255.0);
}

//----------------------------------------------------------------------------
unsigned char QuantizeCoefficient(double coefficient)
{
  const double scaled = std::round((coefficient + 1.0) / 2.0 * 255.0);
  return static_cast<unsigned char>(std::clamp(scaled, 0.0, 255.0));
}

//----------------------------------------------------------------------------
int CoefficientCount(int bands)
{
  constexpr int countByBand[4] = { 0, 3, 8, 15 };
  return countByBand[bands];
}

//----------------------------------------------------------------------------
constexpr const char* ShNames[15] = { "sh1m1", "sh10", "sh1p1", "sh2m2", "sh2m1", "sh20", "sh2p1",
  "sh2p2", "sh3m3", "sh3m2", "sh3m1", "sh30", "sh3p1", "sh3p2", "sh3p3" };

//----------------------------------------------------------------------------
bool DecodeQuaternion(const uint8_t* pixel, std::array<float, 4>& rotation)
{
  const int mode = static_cast<int>(pixel[3]) - 252;
  if (mode < 0 || mode > 3)
  {
    return false;
  }

  const auto component = [](uint8_t channel)
  { return (static_cast<double>(channel) / 255.0 - 0.5) * 2.0 / std::sqrt(2.0); };
  const double a = component(pixel[0]);
  const double b = component(pixel[1]);
  const double c = component(pixel[2]);
  const double omitted = std::sqrt(std::max(0.0, 1.0 - (a * a + b * b + c * c)));

  double wxyz[4] = { 0.0, 0.0, 0.0, 0.0 };
  switch (mode)
  {
    case 0: // omitted w
      wxyz[0] = omitted;
      wxyz[1] = a;
      wxyz[2] = b;
      wxyz[3] = c;
      break;
    case 1: // omitted x
      wxyz[0] = a;
      wxyz[1] = omitted;
      wxyz[2] = b;
      wxyz[3] = c;
      break;
    case 2: // omitted y
      wxyz[0] = a;
      wxyz[1] = b;
      wxyz[2] = omitted;
      wxyz[3] = c;
      break;
    case 3: // omitted z
      wxyz[0] = a;
      wxyz[1] = b;
      wxyz[2] = c;
      wxyz[3] = omitted;
      break;
  }

  for (int i = 0; i < 4; ++i)
  {
    rotation[static_cast<std::size_t>(i)] = static_cast<float>(wxyz[i]);
  }
  return true;
}
}

//----------------------------------------------------------------------------
vtkStandardNewMacro(vtkF3DSOGReader);

//----------------------------------------------------------------------------
int vtkF3DSOGReader::RequestData(
  vtkInformation*, vtkInformationVector**, vtkInformationVector* outputVector)
{
  vtkPolyData* output = vtkPolyData::GetData(outputVector);

  vtkSmartPointer<vtkResourceStream> stream;
  if (this->Stream)
  {
    if (!this->Stream->SupportSeek())
    {
      vtkErrorMacro("Cannot read a SOG file from a non-seekable stream");
      return 0;
    }
    stream = this->Stream;
  }
  else
  {
    vtkNew<vtkFileResourceStream> fileStream;
    if (!fileStream->Open(this->FileName))
    {
      vtkErrorMacro("Cannot open SOG file");
      return 0;
    }
    stream = fileStream;
  }

  vtkNew<vtkF3DArchiveReader> archive;
  if (!archive->Open(stream))
  {
    vtkErrorMacro("Invalid SOG archive");
    return 0;
  }

  std::vector<char> metaBytes;
  if (!archive->Extract("meta.json", metaBytes) || metaBytes.empty())
  {
    vtkErrorMacro("Invalid SOG meta.json");
    return 0;
  }

  nlohmann::json meta;
  try
  {
    meta = nlohmann::json::parse(metaBytes.begin(), metaBytes.end());
  }
  catch (const nlohmann::json::exception&)
  {
    vtkErrorMacro("Invalid SOG meta.json");
    return 0;
  }

  if (!meta.is_object())
  {
    vtkErrorMacro("Invalid SOG meta.json");
    return 0;
  }

  int version = 0;
  try
  {
    version = meta.at("version").get<int>();
  }
  catch (const nlohmann::json::exception&)
  {
    version = 0;
  }
  if (version != 2)
  {
    vtkErrorMacro("Incompatible SOG version. Only version 2 is supported");
    return 0;
  }

  std::int64_t countValue = 0;
  std::vector<std::string> meansFiles;
  std::vector<std::string> scaleFiles;
  std::vector<std::string> quatFiles;
  std::vector<std::string> sh0Files;
  std::vector<std::string> shNFiles;
  std::array<double, 256> scaleCodebook{};
  std::array<double, 256> sh0Codebook{};
  std::array<double, 256> shNCodebook{};
  double mins[3] = { 0.0, 0.0, 0.0 };
  double maxs[3] = { 0.0, 0.0, 0.0 };
  bool hasSH = false;
  int bands = 0;
  std::int64_t shCount = 0;

  try
  {
    if (!meta.contains("count") || !meta["count"].is_number_integer())
    {
      vtkErrorMacro("Invalid SOG meta.json");
      return 0;
    }
    countValue = meta["count"].get<std::int64_t>();
    if (countValue < 0)
    {
      vtkErrorMacro("Invalid SOG meta.json");
      return 0;
    }

    // means files are low byte, then high byte
    if (!meta.contains("means") || !::ReadFiles(meta["means"], 2, meansFiles) ||
      !::ReadRange(meta["means"], "mins", mins) || !::ReadRange(meta["means"], "maxs", maxs) ||
      !meta.contains("scales") || !::ReadFiles(meta["scales"], 1, scaleFiles) ||
      !::ReadCodebook(meta["scales"], scaleCodebook) || !meta.contains("quats") ||
      !::ReadFiles(meta["quats"], 1, quatFiles) || !meta.contains("sh0") ||
      !::ReadFiles(meta["sh0"], 1, sh0Files) || !::ReadCodebook(meta["sh0"], sh0Codebook))
    {
      vtkErrorMacro("Invalid SOG meta.json");
      return 0;
    }

    // shN files are the centroid palette, then the label
    hasSH = meta.contains("shN");
    if (hasSH)
    {
      const nlohmann::json& shN = meta["shN"];
      if (!shN.is_object() || !shN.contains("bands") || !shN["bands"].is_number_integer() ||
        !shN.contains("count") || !shN["count"].is_number_integer())
      {
        vtkErrorMacro("Invalid SOG meta.json");
        return 0;
      }

      bands = shN["bands"].get<int>();
      shCount = shN["count"].get<std::int64_t>();
      if (bands < 1 || bands > 3)
      {
        vtkErrorMacro(
          "Incompatible SOG spherical harmonics. Only degrees 1 through 3 are supported");
        return 0;
      }
      if (shCount < 1 || shCount > 65536 || !::ReadFiles(shN, 2, shNFiles) ||
        !::ReadCodebook(shN, shNCodebook))
      {
        vtkErrorMacro("Invalid SOG meta.json");
        return 0;
      }
    }
  }
  catch (const nlohmann::json::exception&)
  {
    vtkErrorMacro("Invalid SOG meta.json");
    return 0;
  }

  ::Image meansLow;
  ::Image meansHigh;
  ::Image scales;
  ::Image quats;
  ::Image sh0;
  ::Image shCentroids;
  ::Image shLabels;

  const auto readImage = [&](const std::string& name, ::Image& image)
  {
    if (!::ReadImage(archive, name, image))
    {
      vtkErrorMacro("Cannot read SOG image \"" << name << "\"");
      return false;
    }
    return true;
  };

  if (!readImage(meansFiles[0], meansLow) || !readImage(meansFiles[1], meansHigh) ||
    !readImage(scaleFiles[0], scales) || !readImage(quatFiles[0], quats) ||
    !readImage(sh0Files[0], sh0) ||
    (hasSH && (!readImage(shNFiles[0], shCentroids) || !readImage(shNFiles[1], shLabels))))
  {
    return 0;
  }

  const int width = ::Width(meansLow);
  const int height = ::Height(meansLow);
  if (::Width(meansHigh) != width || ::Height(meansHigh) != height || ::Width(scales) != width ||
    ::Height(scales) != height || ::Width(quats) != width || ::Height(quats) != height ||
    ::Width(sh0) != width || ::Height(sh0) != height ||
    (hasSH && (::Width(shLabels) != width || ::Height(shLabels) != height)))
  {
    vtkErrorMacro("SOG images do not share one width and height");
    return 0;
  }

  const std::int64_t capacity =
    static_cast<std::int64_t>(width) * static_cast<std::int64_t>(height);
  if (countValue > capacity)
  {
    vtkErrorMacro("SOG count exceeds width * height");
    return 0;
  }

  const vtkIdType count = static_cast<vtkIdType>(countValue);

  vtkNew<vtkFloatArray> positionArray;
  positionArray->SetNumberOfComponents(3);
  positionArray->SetNumberOfTuples(count);
  positionArray->SetName("position");

  vtkNew<vtkUnsignedCharArray> colorArray;
  colorArray->SetNumberOfComponents(4);
  colorArray->SetNumberOfTuples(count);
  colorArray->SetName("color");

  vtkNew<vtkFloatArray> scaleArray;
  scaleArray->SetNumberOfComponents(3);
  scaleArray->SetNumberOfTuples(count);
  scaleArray->SetName("scale");

  vtkNew<vtkFloatArray> rotationArray;
  rotationArray->SetNumberOfComponents(4);
  rotationArray->SetNumberOfTuples(count);
  rotationArray->SetName("rotation");

  const int numCoeffs = hasSH ? ::CoefficientCount(bands) : 0;
  vtkNew<vtkUnsignedCharArray> shArrays[15];
  for (int coefficient = 0; coefficient < numCoeffs; ++coefficient)
  {
    shArrays[coefficient]->SetNumberOfComponents(3);
    shArrays[coefficient]->SetNumberOfTuples(count);
    shArrays[coefficient]->SetName(::ShNames[coefficient]);
  }

  for (vtkIdType splatIndex = 0; splatIndex < count; ++splatIndex)
  {
    const uint8_t* low = ::Pixel(meansLow, splatIndex);
    const uint8_t* high = ::Pixel(meansHigh, splatIndex);
    const float position[3] = { ::DecodePosition(low[0], high[0], mins[0], maxs[0]),
      ::DecodePosition(low[1], high[1], mins[1], maxs[1]),
      ::DecodePosition(low[2], high[2], mins[2], maxs[2]) };
    positionArray->SetTypedTuple(splatIndex, position);

    const uint8_t* scalePixel = ::Pixel(scales, splatIndex);
    const float scale[3] = { ::DecodeScale(scaleCodebook[scalePixel[0]]),
      ::DecodeScale(scaleCodebook[scalePixel[1]]), ::DecodeScale(scaleCodebook[scalePixel[2]]) };
    scaleArray->SetTypedTuple(splatIndex, scale);

    const uint8_t* colorPixel = ::Pixel(sh0, splatIndex);
    const unsigned char color[4] = { ::DecodeColor(sh0Codebook[colorPixel[0]]),
      ::DecodeColor(sh0Codebook[colorPixel[1]]), ::DecodeColor(sh0Codebook[colorPixel[2]]),
      colorPixel[3] };
    colorArray->SetTypedTuple(splatIndex, color);

    std::array<float, 4> rotation;
    if (!::DecodeQuaternion(::Pixel(quats, splatIndex), rotation))
    {
      vtkErrorMacro("Invalid SOG quaternion");
      return 0;
    }
    rotationArray->SetTypedTuple(splatIndex, rotation.data());

    if (hasSH)
    {
      const uint8_t* labelPixel = ::Pixel(shLabels, splatIndex);
      const unsigned int label =
        static_cast<unsigned int>(labelPixel[0]) + (static_cast<unsigned int>(labelPixel[1]) << 8);
      const int xOrigin = static_cast<int>(label % 64) * numCoeffs;
      const int y = static_cast<int>(label / 64);
      if (label >= static_cast<unsigned int>(shCount) || y < 0 || y >= ::Height(shCentroids) ||
        xOrigin < 0 || xOrigin + numCoeffs > ::Width(shCentroids))
      {
        vtkErrorMacro("Invalid SOG spherical harmonics");
        return 0;
      }

      for (int coefficient = 0; coefficient < numCoeffs; ++coefficient)
      {
        const vtkIdType centroidIndex = static_cast<vtkIdType>(y) * ::Width(shCentroids) +
          static_cast<vtkIdType>(xOrigin + coefficient);
        const uint8_t* centroid = ::Pixel(shCentroids, centroidIndex);
        const unsigned char quantized[3] = { ::QuantizeCoefficient(shNCodebook[centroid[0]]),
          ::QuantizeCoefficient(shNCodebook[centroid[1]]),
          ::QuantizeCoefficient(shNCodebook[centroid[2]]) };
        shArrays[coefficient]->SetTypedTuple(splatIndex, quantized);
      }
    }
  }

  vtkNew<vtkPoints> points;
  points->SetDataTypeToFloat();
  points->SetData(positionArray);
  output->SetPoints(points);

  output->GetPointData()->SetScalars(colorArray);
  output->GetPointData()->AddArray(scaleArray);
  output->GetPointData()->AddArray(rotationArray);
  for (int coefficient = 0; coefficient < numCoeffs; ++coefficient)
  {
    output->GetPointData()->AddArray(shArrays[coefficient]);
  }
  return 1;
}

//----------------------------------------------------------------------------
bool vtkF3DSOGReader::CanReadFile(vtkResourceStream* stream)
{
  if (!stream || !stream->SupportSeek())
  {
    return false;
  }

  const vtkTypeInt64 position = stream->Tell();
  bool canRead = false;

  stream->Seek(0, vtkResourceStream::SeekDirection::Begin);
  char magic[4];
  if (stream->Read(magic, sizeof(magic)) == sizeof(magic) && magic[0] == 'P' && magic[1] == 'K' &&
    magic[2] == '\x03' && magic[3] == '\x04')
  {
    vtkNew<vtkF3DArchiveReader> archive;
    if (archive->Open(stream))
    {
      canRead = archive->Has("meta.json");
    }
  }

  stream->Seek(position, vtkResourceStream::SeekDirection::Begin);
  return canRead;
}

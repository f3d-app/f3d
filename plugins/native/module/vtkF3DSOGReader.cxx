#include "vtkF3DSOGReader.h"

#include "vtkF3DArchiveReader.h"

#include <vtkFileResourceStream.h>
#include <vtkFloatArray.h>
#include <vtkInformation.h>
#include <vtkInformationVector.h>
#include <vtkNew.h>
#include <vtkObjectFactory.h>
#include <vtkPointData.h>
#include <vtkPoints.h>
#include <vtkPolyData.h>
#include <vtkResourceStream.h>
#include <vtkSmartPointer.h>
#include <vtkUnsignedCharArray.h>

#include <nlohmann/json.hpp>
#include <webp/decode.h>

#include <algorithm>
#include <array>
#include <cmath>
#include <cstdint>
#include <string>
#include <vector>

namespace
{
struct SogImage
{
  int Width = 0;
  int Height = 0;
  std::vector<uint8_t> Pixels;
};

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
bool ReadImage(vtkF3DArchiveReader* archive, const std::string& name, SogImage& image)
{
  std::vector<char> bytes;
  if (!archive->Extract(name, bytes) || bytes.empty())
  {
    return false;
  }

  int width = 0;
  int height = 0;
  uint8_t* decoded = WebPDecodeRGBA(
    reinterpret_cast<const uint8_t*>(bytes.data()), bytes.size(), &width, &height);
  if (!decoded || width <= 0 || height <= 0)
  {
    if (decoded)
    {
      WebPFree(decoded);
    }
    return false;
  }

  const std::size_t byteCount =
    static_cast<std::size_t>(width) * static_cast<std::size_t>(height) * 4;
  image.Width = width;
  image.Height = height;
  image.Pixels.assign(decoded, decoded + byteCount);
  WebPFree(decoded);
  return true;
}

//----------------------------------------------------------------------------
const uint8_t* Pixel(const SogImage& image, vtkIdType index)
{
  return image.Pixels.data() + static_cast<std::size_t>(index) * 4;
}

//----------------------------------------------------------------------------
// Section 3.1 Positions. 16 bits per axis, then sign(n) * (exp(|n|) - 1).
float DecodePosition(uint8_t low, uint8_t high, double minimum, double maximum)
{
  const unsigned int quantized = (static_cast<unsigned int>(high) << 8) | low;
  const double normalized =
    minimum + (maximum - minimum) * (static_cast<double>(quantized) / 65535.0);
  const double magnitude = std::exp(std::abs(normalized)) - 1.0;
  return static_cast<float>(std::copysign(magnitude, normalized));
}

//----------------------------------------------------------------------------
// Section 3.3 Scales. The codebook is log-domain.
float DecodeScale(double coefficient)
{
  return static_cast<float>(std::exp(coefficient));
}

//----------------------------------------------------------------------------
// Section 3.4 Base color. SH_C0 = 1 / (2 * sqrt(pi)). Truncate to a byte, as SPZ does.
unsigned char DecodeColor(double coefficient)
{
  constexpr double shC0 = 0.28209479177387814;
  const double color = std::clamp(0.5 + coefficient * shC0, 0.0, 1.0);
  return static_cast<unsigned char>(color * 255.0);
}

//----------------------------------------------------------------------------
// Section 3.2 Version 2 smallest-three quaternion, stored (w, x, y, z).
bool DecodeQuaternion(const uint8_t* pixel, std::array<float, 4>& rotation)
{
  const int mode = static_cast<int>(pixel[3]) - 252;
  if (mode < 0 || mode > 3)
  {
    return false;
  }

  const auto component = [](uint8_t channel)
  {
    return (static_cast<double>(channel) / 255.0 - 0.5) * 2.0 / std::sqrt(2.0);
  };
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
  std::array<double, 256> scaleCodebook{};
  std::array<double, 256> sh0Codebook{};
  double mins[3] = { 0.0, 0.0, 0.0 };
  double maxs[3] = { 0.0, 0.0, 0.0 };

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

    // Names come from each files array. Order is significant: means files are
    // low byte, then high byte. The names themselves are not fixed.
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
  }
  catch (const nlohmann::json::exception&)
  {
    vtkErrorMacro("Invalid SOG meta.json");
    return 0;
  }

  ::SogImage meansLow;
  ::SogImage meansHigh;
  ::SogImage scales;
  ::SogImage quats;
  ::SogImage sh0;

  const auto readImage = [&](const std::string& name, ::SogImage& image)
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
    !readImage(sh0Files[0], sh0))
  {
    return 0;
  }

  const int width = meansLow.Width;
  const int height = meansLow.Height;
  if (meansHigh.Width != width || meansHigh.Height != height || scales.Width != width ||
    scales.Height != height || quats.Width != width || quats.Height != height ||
    sh0.Width != width || sh0.Height != height)
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

  for (vtkIdType splatIndex = 0; splatIndex < count; ++splatIndex)
  {
    // Same order WebPDecodeRGBA returns: x = i % width, y = i / width, top row first.
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

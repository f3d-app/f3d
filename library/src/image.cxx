#include "image.h"

#include "export.h"
#include "init.h"

#include <vtkBMPWriter.h>
#include <vtkDataArrayRange.h>
#include <vtkDoubleArray.h>
#include <vtkImageData.h>
#include <vtkImageReader2.h>
#include <vtkImageReader2Collection.h>
#include <vtkImageReader2Factory.h>
#include <vtkJPEGWriter.h>
#include <vtkPNGReader.h>
#include <vtkPNGWriter.h>
#include <vtkPointData.h>
#include <vtkSmartPointer.h>
#include <vtkStringArray.h>
#include <vtkTIFFWriter.h>
#include <vtkUnsignedCharArray.h>
#include <vtkVersion.h>
#include <vtksys/SystemTools.hxx>

#if VTK_VERSION_NUMBER >= VTK_VERSION_CHECK(9, 3, 20240729)
#include <vtkImageSSIM.h>
#else
#include <vtkImageDifference.h>
#endif

#include <algorithm>
#include <cassert>
#include <regex>
#include <sstream>
#include <string>
#include <unordered_map>

namespace fs = std::filesystem;

namespace f3d
{
class image::internals
{
public:
  inline static const std::string metadataKeyPrefix = "f3d:";
  inline static const std::map<SaveFormat, std::string> saveFormatString = {
    { SaveFormat::PNG, "PNG" },
    { SaveFormat::BMP, "BMP" },
    { SaveFormat::JPG, "JPG" },
    { SaveFormat::TIF, "TIF" },
  };

  vtkSmartPointer<vtkImageData> Image;
  std::unordered_map<std::string, std::string> Metadata;

  template<typename WriterType>
  std::vector<unsigned char> SaveBuffer(vtkSmartPointer<WriterType> writer)
  {
    writer->WriteToMemoryOn();
    writer->SetInputData(this->Image);
    writer->Write();

    std::vector<unsigned char> result;

    auto valRange = vtk::DataArrayValueRange(writer->GetResult());
    std::copy(valRange.begin(), valRange.end(), std::back_inserter(result));

    return result;
  }

  void WritePngMetadata(vtkPNGWriter* pngWriter)
  {
    // cppcheck-suppress unassignedVariable
    // (false positive, fixed in cppcheck 2.8)
    for (const auto& [key, value] : this->Metadata)
    {
      if (!value.empty())
      {
        pngWriter->AddText((metadataKeyPrefix + key).c_str(), value.c_str());
      }
    }
  }

  void ReadPngMetadata(vtkPNGReader* pngReader)
  {
    int beginEndIndex[2];
    for (size_t i = 0; i < pngReader->GetNumberOfTextChunks(); ++i)
    {
      const vtkStdString key = pngReader->GetTextKey(static_cast<int>(i));
      if (key.rfind(metadataKeyPrefix, 0) == 0)
      {
        pngReader->GetTextChunks(key.c_str(), beginEndIndex);
        const int index = beginEndIndex[1] - 1; // only read the last key
        if (index > -1)
        {
          const std::string value(pngReader->GetTextValue(index));
          if (!value.empty())
          {
            this->Metadata[key.substr(metadataKeyPrefix.length())] = value;
          }
        }
      }
    }
  }

  static void checkSaveFormatCompatibility(const image& self, SaveFormat format)
  {
    ChannelType type = self.getChannelType();
    int count = self.getChannelCount();

    switch (format)
    {
      case SaveFormat::PNG:
        if (type != ChannelType::BYTE && type != ChannelType::SHORT)
        {
          throw write_exception("PNG format is only compatible with BYTE or SHORT channel types");
        }
        break;
      case SaveFormat::JPG:
      case SaveFormat::BMP:
        if (type != ChannelType::BYTE)
        {
          throw write_exception(
            saveFormatString.at(format) + " format is only compatible with BYTE channel types");
        }
        break;
      default:
        break;
    }

    switch (format)
    {
      case SaveFormat::JPG:
        if (count != 1 && count != 3)
        {
          throw write_exception("JPG format is only compatible with a channel count of 1 or 3");
        }
        break;
      case SaveFormat::PNG:
      case SaveFormat::BMP:
      case SaveFormat::TIF:
        if (count < 1 || count > 4)
        {
          throw write_exception(saveFormatString.at(format) +
            " format is only compatible with a channel count between 1 to 4");
        }
        break;
    }
  }
};

//----------------------------------------------------------------------------
image::image()
  : Internals(new image::internals())
{
  this->Internals->Image = vtkSmartPointer<vtkImageData>::New();
}

//----------------------------------------------------------------------------
image::image(unsigned int width, unsigned int height, unsigned int channelCount, ChannelType type)
  : Internals(new image::internals())
{
  this->Internals->Image = vtkSmartPointer<vtkImageData>::New();
  this->Internals->Image->SetDimensions(static_cast<int>(width), static_cast<int>(height), 1);

  switch (type)
  {
    case ChannelType::BYTE:
      this->Internals->Image->AllocateScalars(VTK_UNSIGNED_CHAR, static_cast<int>(channelCount));
      break;
    case ChannelType::SHORT:
      this->Internals->Image->AllocateScalars(VTK_UNSIGNED_SHORT, static_cast<int>(channelCount));
      break;
    case ChannelType::FLOAT:
      this->Internals->Image->AllocateScalars(VTK_FLOAT, static_cast<int>(channelCount));
      break;
  }
}

//----------------------------------------------------------------------------
image::image(const fs::path& filePath)
  : Internals(new image::internals())
{
  detail::init::initialize();

  try
  {
    if (!fs::exists(filePath))
    {
      throw read_exception("Cannot open file " + filePath.string());
    }

    auto reader = vtkSmartPointer<vtkImageReader2>::Take(
      vtkImageReader2Factory::CreateImageReader2(filePath.string().c_str()));

    if (reader)
    {
      reader->SetFileName(filePath.string().c_str());
      reader->Update();
      this->Internals->Image = reader->GetOutput();

      vtkPNGReader* pngReader = vtkPNGReader::SafeDownCast(reader);
      if (pngReader != nullptr)
      {
        this->Internals->ReadPngMetadata(pngReader);
      }
    }

    if (!this->Internals->Image)
    {
      throw read_exception("Cannot read image " + filePath.string());
    }
  }
  catch (const fs::filesystem_error& ex)
  {
    throw read_exception(std::string("Cannot read image: ") + ex.what());
  }
}

//----------------------------------------------------------------------------
image::~image()
{
  delete this->Internals;
}

//----------------------------------------------------------------------------
image::image(const image& img)
  : Internals(new image::internals())
{
  this->Internals->Image = vtkSmartPointer<vtkImageData>::New();
  this->Internals->Image->DeepCopy(img.Internals->Image);
}

//----------------------------------------------------------------------------
image& image::operator=(const image& img) noexcept
{
  if (this != &img)
  {
    this->Internals->Image = vtkSmartPointer<vtkImageData>::New();
    this->Internals->Image->DeepCopy(img.Internals->Image);
  }
  return *this;
}

//----------------------------------------------------------------------------
image::image(image&& img) noexcept
  : Internals(nullptr)
{
  std::swap(this->Internals, img.Internals);
}

//----------------------------------------------------------------------------
image& image::operator=(image&& img) noexcept
{
  std::swap(this->Internals, img.Internals);
  return *this;
}

//----------------------------------------------------------------------------
std::vector<std::string> image::getSupportedFormats()
{
  std::vector<std::string> formats;

  vtkNew<vtkImageReader2Collection> collection;
  vtkImageReader2Factory::GetRegisteredReaders(collection);

  collection->InitTraversal();
  vtkImageReader2* reader = collection->GetNextItem();

  while (reader != nullptr)
  {
    std::string extensions = reader->GetFileExtensions();

    std::regex re("\\s+");
    std::sregex_token_iterator first{ extensions.begin(), extensions.end(), re, -1 }, last;

    std::copy(first, last, std::back_inserter(formats));

    reader = collection->GetNextItem();
  }

  return formats;
}

//----------------------------------------------------------------------------
unsigned int image::getWidth() const
{
  int dims[3];
  this->Internals->Image->GetDimensions(dims);
  return dims[0];
}

//----------------------------------------------------------------------------
unsigned int image::getHeight() const
{
  int dims[3];
  this->Internals->Image->GetDimensions(dims);
  return dims[1];
}

//----------------------------------------------------------------------------
unsigned int image::getChannelCount() const
{
  return this->Internals->Image->GetNumberOfScalarComponents();
}

//----------------------------------------------------------------------------
image::ChannelType image::getChannelType() const
{
  switch (this->Internals->Image->GetScalarType())
  {
    case VTK_UNSIGNED_CHAR:
      return ChannelType::BYTE;
    case VTK_UNSIGNED_SHORT:
      return ChannelType::SHORT;
    case VTK_FLOAT:
      return ChannelType::FLOAT;
    default:
      break;
  }
  throw read_exception("Unknown channel type");
}

//----------------------------------------------------------------------------
unsigned int image::getChannelTypeSize() const
{
  return this->Internals->Image->GetScalarSize();
}

//----------------------------------------------------------------------------
image& image::setContent(void* buffer)
{
  unsigned int scalarSize = this->Internals->Image->GetScalarSize();
  unsigned int totalSize =
    this->getWidth() * this->getHeight() * this->getChannelCount() * scalarSize;
  uint8_t* internalBuffer = static_cast<uint8_t*>(this->Internals->Image->GetScalarPointer());
  std::copy_n(static_cast<uint8_t*>(buffer), totalSize, internalBuffer);
  return *this;
}

//----------------------------------------------------------------------------
void* image::getContent() const
{
  return this->Internals->Image->GetScalarPointer();
}

//----------------------------------------------------------------------------
bool image::compare(const image& reference, double threshold, double& error) const
{
  // Sanity check for threshold
  if (threshold < 0 || threshold >= 1)
  {
    error = 1;
    return false;
  }

  ChannelType type = this->getChannelType();
  if (type != reference.getChannelType())
  {
    error = 1;
    return false;
  }

  unsigned int count = this->getChannelCount();
  if (count != reference.getChannelCount())
  {
    error = 1;
    return false;
  }

  if (this->getWidth() != reference.getWidth() || this->getHeight() != reference.getHeight())
  {
    error = 1;
    return false;
  }

  if (this->getWidth() == 0 && this->getHeight() == 0)
  {
    error = 0;
    return true;
  }

#if VTK_VERSION_NUMBER >= VTK_VERSION_CHECK(9, 3, 20240729)
  vtkNew<vtkImageSSIM> ssim;
  std::vector<int> ranges(count);
  switch (type)
  {
    case ChannelType::BYTE:
      std::fill(ranges.begin(), ranges.end(), 256);
      ssim->SetInputRange(ranges);
      break;
    case ChannelType::SHORT:
      std::fill(ranges.begin(), ranges.end(), 65535);
      ssim->SetInputRange(ranges);
      break;
    case ChannelType::FLOAT:
      ssim->SetInputToAuto();
      break;
  }

  ssim->SetInputData(this->Internals->Image);
  ssim->SetInputData(1, reference.Internals->Image);
  ssim->Update();
  vtkDoubleArray* scalars = vtkArrayDownCast<vtkDoubleArray>(
    vtkDataSet::SafeDownCast(ssim->GetOutputDataObject(0))->GetPointData()->GetScalars());

  // Thanks to the checks above, this is always true
  assert(scalars != nullptr);

  double unused;
  vtkImageSSIM::ComputeErrorMetrics(scalars, error, unused);
  return error <= threshold;
#else
  threshold *= 1000;

  vtkNew<vtkImageDifference> imDiff;
  imDiff->SetThreshold(0);
  imDiff->SetInputData(this->Internals->Image);
  imDiff->SetImageData(reference.Internals->Image);
  imDiff->UpdateInformation();
  error = imDiff->GetThresholdedError();

  if (error <= threshold)
  {
    imDiff->Update();
    error = imDiff->GetThresholdedError();
  }

  bool ret = error <= threshold;
  error /= 1000;
  return ret;
#endif
}

//----------------------------------------------------------------------------
bool image::operator==(const image& reference) const
{
  double error;
  // XXX: We do not use 0 because even with identical images, rounding error, arithmetic imprecision
  // or architecture issue may cause the value to not be 0. See:
  // https://develop.openfoam.com/Development/openfoam/-/issues/2958
  return this->compare(reference, 1e-14, error);
}

//----------------------------------------------------------------------------
bool image::operator!=(const image& reference) const
{
  return !this->operator==(reference);
}

//----------------------------------------------------------------------------
std::vector<double> image::getNormalizedPixel(const std::pair<int, int>& xy) const
{
  std::vector<double> pixel(this->getChannelCount());

  for (size_t i = 0; i < pixel.size(); i++)
  {
    double v = this->Internals->Image->GetScalarComponentAsDouble(
      xy.first, xy.second, 0, static_cast<int>(i));

    switch (this->getChannelType())
    {
      case ChannelType::BYTE:
        pixel[i] = v / 255.0;
        break;
      case ChannelType::SHORT:
        pixel[i] = v / 65535.0;
        break;
      default:
        pixel[i] = v;
        break;
    }
  }

  return pixel;
}

//----------------------------------------------------------------------------
const image& image::save(const fs::path& filePath, SaveFormat format) const
{
  internals::checkSaveFormatCompatibility(*this, format);

  vtkSmartPointer<vtkImageWriter> writer;
  switch (format)
  {
    case SaveFormat::PNG:
    {
      vtkNew<vtkPNGWriter> pngWriter;
      this->Internals->WritePngMetadata(pngWriter);
      writer = pngWriter;
    }
    break;
    case SaveFormat::JPG:
      writer = vtkSmartPointer<vtkJPEGWriter>::New();
      break;
    case SaveFormat::TIF:
      writer = vtkSmartPointer<vtkTIFFWriter>::New();
      break;
    case SaveFormat::BMP:
      writer = vtkSmartPointer<vtkBMPWriter>::New();
      break;
  }

  try
  {
    // Ensure the directories exists
    fs::create_directories(filePath.parent_path());

    writer->SetFileName(filePath.string().c_str());
    writer->SetInputData(this->Internals->Image);
    writer->Write();

    if (writer->GetErrorCode() != 0)
    {
      throw write_exception("Cannot write " + filePath.string());
    }
  }
  catch (const fs::filesystem_error& ex)
  {
    throw write_exception(std::string("Cannot write image: ") + ex.what());
  }

  return *this;
}

//----------------------------------------------------------------------------
std::vector<unsigned char> image::saveBuffer(SaveFormat format) const
{
  internals::checkSaveFormatCompatibility(*this, format);

  switch (format)
  {
    case SaveFormat::PNG:
    {
      vtkSmartPointer<vtkPNGWriter> writer = vtkSmartPointer<vtkPNGWriter>::New();
      this->Internals->WritePngMetadata(writer);
      return this->Internals->SaveBuffer(writer);
    }
    case SaveFormat::JPG:
      return this->Internals->SaveBuffer(vtkSmartPointer<vtkJPEGWriter>::New());
    case SaveFormat::BMP:
      return this->Internals->SaveBuffer(vtkSmartPointer<vtkBMPWriter>::New());
    default:
      throw write_exception(
        "Cannot save to buffer in the specified format: " + internals::saveFormatString.at(format));
  }
}

//----------------------------------------------------------------------------
const image& image::toTerminalText(std::ostream& stream) const
{
  const int depth = this->getChannelCount();
  if (this->getChannelType() != ChannelType::BYTE || depth < 3 || depth > 4)
  {
    throw write_exception("image must be byte RGB or RGBA");
  }

  int dims[3];
  this->Internals->Image->GetDimensions(dims);
  const int width = dims[0];
  const int height = dims[1];
  const unsigned char* content = static_cast<unsigned char*>(this->getContent());

  constexpr unsigned char alphaCutoff = 127;

  /* Function to retrieve pixels so we can return:
    - transparent black values for out-of-bounds coords,
    - opaque alpha value when the image has no alpha channel.
    Rendering with half blocks means 1 line of text represents 2 rows of pixels
    so we _will_ attempt to access a line past the bottom if the height is not even.
  */
  const auto getPixel = [=](int x, int y)
  {
    if (x >= 0 && x < width && y >= 0 && y < height)
    {
      const size_t i = depth * ((height - 1 - y) * width + x);
      const int rgb = content[i + 0] << 16 | content[i + 1] << 8 | content[i + 2];
      const bool transparent = depth > 3 ? content[i + 3] <= alphaCutoff : false;
      return std::make_pair(rgb, transparent);
    }
    return std::make_pair(0x000000, true);
  };

  /* Functions to manipulate the terminal colors using escape sequences.
    Keep track of the foreground and background states to avoid redundant sequences.
  */
  int currentFg = -1;
  int currentBg = -1;
  const auto setFg = [&](int rgb)
  {
    if (currentFg != rgb)
    {
      stream << "\033[38;2;" // set 24-bit foreground
             << ((rgb >> 16) & 0xff) << ";" << ((rgb >> 8) & 0xff) << ";" << (rgb & 0xff) << "m";
      currentFg = rgb;
    }
  };
  const auto setBg = [&](int rgb)
  {
    if (currentBg != rgb)
    {
      stream << "\033[48;2;" // set 24-bit background
             << ((rgb >> 16) & 0xff) << ";" << ((rgb >> 8) & 0xff) << ";" << (rgb & 0xff) << "m";
      currentBg = rgb;
    }
  };
  const auto reset = [&]()
  {
    if (currentBg > -1 || currentFg > -1)
    {
      stream << "\033[0m"; // reset all
      currentBg = -1;
      currentFg = -1;
    }
  };
  const auto resetBg = [&]()
  {
    if (currentBg > -1)
    {
      stream << "\033[49m"; // reset background
      currentBg = -1;
    }
  };

  constexpr std::string_view EMPTY_BLOCK = " ";
  constexpr std::string_view TOP_BLOCK = u8"\u2580";
  constexpr std::string_view BOTTOM_BLOCK = u8"\u2584";
  constexpr std::string_view FULL_BLOCK = u8"\u2588";
  constexpr std::string_view EOL = "\n";

  for (int y = 0; y < height; y += 2)
  {
    if (y > 0)
    {
      stream << EOL;
    }
    for (int x = 0; x < width; ++x)
    {
      const auto [rgb1, blank1] = getPixel(x, y + 0);
      const auto [rgb2, blank2] = getPixel(x, y + 1);
      if (blank1 && blank2)
      {
        reset();
        stream << EMPTY_BLOCK;
      }
      else if (blank1)
      {
        resetBg();
        setFg(rgb2);
        stream << BOTTOM_BLOCK;
      }
      else if (blank2)
      {
        resetBg();
        setFg(rgb1);
        stream << TOP_BLOCK;
      }
      else if (rgb1 == rgb2)
      {
        setFg(rgb1);
        stream << FULL_BLOCK;
      }
      else if (rgb1 == currentFg || rgb2 == currentBg)
      {
        setBg(rgb2);
        setFg(rgb1);
        stream << TOP_BLOCK;
      }
      else
      {
        setBg(rgb1);
        setFg(rgb2);
        stream << BOTTOM_BLOCK;
      }
    }
    reset(); // reset after every line to keep the right edge of the image
  }

  return *this;
}

//----------------------------------------------------------------------------
std::string image::toTerminalText() const
{
  std::stringstream ss;
  toTerminalText(ss);
  return ss.str();
}

//----------------------------------------------------------------------------
image& image::setMetadata(std::string key, std::string value)
{
  if (value.empty())
  {
    this->Internals->Metadata.erase(key);
  }
  else
  {
    this->Internals->Metadata[std::move(key)] = std::move(value);
  }
  return *this;
}

//----------------------------------------------------------------------------
std::string image::getMetadata(const std::string& key) const
{
  if (this->Internals->Metadata.count(key))
  {
    return this->Internals->Metadata[key];
  }
  throw metadata_exception("No such key: " + key);
}

//----------------------------------------------------------------------------
std::vector<std::string> image::allMetadata() const
{
  std::vector<std::string> keys;
  std::transform(this->Internals->Metadata.begin(), this->Internals->Metadata.end(),
    std::back_inserter(keys), [](const auto& kv) { return kv.first; });
  return keys;
}

//----------------------------------------------------------------------------
image::write_exception::write_exception(const std::string& what)
  : exception(what)
{
}

//----------------------------------------------------------------------------
image::read_exception::read_exception(const std::string& what)
  : exception(what)
{
}

//----------------------------------------------------------------------------
image::metadata_exception::metadata_exception(const std::string& what)
  : exception(what)
{
}

}

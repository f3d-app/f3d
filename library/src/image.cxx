#include "image.h"

#include "export.h"
#include "init.h"

#include <vtkBMPWriter.h>
#include <vtkDataArrayRange.h>
#include <vtkImageData.h>
#include <vtkImageDifference.h>
#include <vtkImageReader2.h>
#include <vtkImageReader2Collection.h>
#include <vtkImageReader2Factory.h>
#include <vtkJPEGWriter.h>
#include <vtkPNGWriter.h>
#include <vtkPointData.h>
#include <vtkSmartPointer.h>
#include <vtkTIFFWriter.h>
#include <vtkUnsignedCharArray.h>
#include <vtksys/SystemTools.hxx>

#include <cassert>
#include <regex>
#include <sstream>
#include <string>

namespace f3d
{
class image::internals
{
public:
  vtkSmartPointer<vtkImageData> Image;

  template<typename WriterType>
  std::vector<unsigned char> SaveBuffer()
  {
    vtkNew<WriterType> writer;
    writer->WriteToMemoryOn();
    writer->SetInputData(this->Image);
    writer->Write();

    std::vector<unsigned char> result;

    auto valRange = vtk::DataArrayValueRange(writer->GetResult());
    std::copy(valRange.begin(), valRange.end(), std::back_inserter(result));

    return result;
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
image::image(const std::string& path)
  : Internals(new image::internals())
{
  detail::init::initialize();

  std::string fullPath = vtksys::SystemTools::CollapseFullPath(path);
  if (!vtksys::SystemTools::FileExists(fullPath))
  {
    throw read_exception("Cannot open file " + path);
  }

  auto reader = vtkSmartPointer<vtkImageReader2>::Take(
    vtkImageReader2Factory::CreateImageReader2(fullPath.c_str()));

  if (reader)
  {
    reader->SetFileName(fullPath.c_str());
    reader->Update();
    this->Internals->Image = reader->GetOutput();
  }

  if (!this->Internals->Image)
  {
    throw read_exception("Cannot read image " + path);
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
image::image(image&& img) noexcept : Internals(nullptr)
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

#ifndef F3D_NO_DEPRECATED
//----------------------------------------------------------------------------
image& image::setResolution(unsigned int width, unsigned int height)
{
  this->Internals->Image->SetDimensions(static_cast<int>(width), static_cast<int>(height), 1);
  this->Internals->Image->AllocateScalars(
    VTK_UNSIGNED_CHAR, static_cast<int>(this->getChannelCount()));
  return *this;
}
#endif

//----------------------------------------------------------------------------
unsigned int image::getChannelCount() const
{
  return this->Internals->Image->GetNumberOfScalarComponents();
}

#ifndef F3D_NO_DEPRECATED
//----------------------------------------------------------------------------
image& image::setChannelCount(unsigned int dim)
{
  this->Internals->Image->AllocateScalars(VTK_UNSIGNED_CHAR, static_cast<int>(dim));
  return *this;
}
#endif

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

#ifndef F3D_NO_DEPRECATED
//----------------------------------------------------------------------------
image& image::setData(unsigned char* buffer)
{
  this->setContent(buffer);
  return *this;
}

//----------------------------------------------------------------------------
unsigned char* image::getData() const
{
  return static_cast<unsigned char*>(this->getContent());
}
#endif

//----------------------------------------------------------------------------
bool image::compare(const image& reference, double threshold, image& diff, double& error) const
{
  vtkNew<vtkImageDifference> imDiff;
  // handle threshold outside of vtkImageDifference:
  // https://gitlab.kitware.com/vtk/vtk/-/issues/18152
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

  if (error > threshold)
  {
    imDiff->Update();
    diff.Internals->Image = imDiff->GetOutput();
    return false;
  }

  return true;
}

//----------------------------------------------------------------------------
bool image::operator==(const image& reference) const
{
  image diff;
  double error;
  return this->compare(reference, 0, diff, error);
}

//----------------------------------------------------------------------------
bool image::operator!=(const image& reference) const
{
  return !this->operator==(reference);
}

//----------------------------------------------------------------------------
void image::save(const std::string& path, SaveFormat format) const
{
  vtkSmartPointer<vtkImageWriter> writer;

  switch (format)
  {
    case SaveFormat::PNG:
      writer = vtkSmartPointer<vtkPNGWriter>::New();
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

  writer->SetFileName(path.c_str());
  writer->SetInputData(this->Internals->Image);
  writer->Write();

  if (writer->GetErrorCode() != 0)
  {
    throw write_exception("Cannot write " + path);
  }
}

//----------------------------------------------------------------------------
std::vector<unsigned char> image::saveBuffer(SaveFormat format) const
{
  switch (format)
  {
    case SaveFormat::PNG:
      return this->Internals->SaveBuffer<vtkPNGWriter>();
    case SaveFormat::JPG:
      return this->Internals->SaveBuffer<vtkJPEGWriter>();
    case SaveFormat::BMP:
      return this->Internals->SaveBuffer<vtkBMPWriter>();
    default:
      throw write_exception("Cannot save to buffer in the specified format");
  }
}

//----------------------------------------------------------------------------
const f3d::image& image::toTerminalText(std::ostream& stream) const
{
  const int depth = this->getChannelCount();
  if (this->getChannelType() != ChannelType::BYTE || depth < 3 || depth > 4)
  {
    throw std::invalid_argument("image must be byte RGB or RGBA");
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
  // clang-format off
  constexpr std::string_view TOP_BLOCK = u8"\u2580";
  constexpr std::string_view BOTTOM_BLOCK = u8"\u2584";
  constexpr std::string_view FULL_BLOCK = u8"\u2588";
  // clang-format on
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
image::write_exception::write_exception(const std::string& what)
  : exception(what)
{
}

//----------------------------------------------------------------------------
image::read_exception::read_exception(const std::string& what)
  : exception(what)
{
}

}

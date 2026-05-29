#include "../plugin/reader.h"
#include "../public/scene.h"

namespace f3d
{
bool reader::canRead(const std::string& fileName, const std::optional<bool> skipContentCheck,
    f3d::file_availability& availability) const
{
  std::string ext = fileName.substr(fileName.find_last_of(".") + 1);
  std::transform(ext.begin(), ext.end(), ext.begin(), ::tolower);

  const std::vector<std::string>& extensions = this->getExtensions();

  if (std::any_of(
        extensions.begin(), extensions.end(), [&](const std::string& s) { return s == ext; }))
  {
    vtkNew<vtkFileResourceStream> stream;
    if (skipContentCheck.has_value() && skipContentCheck.value() == true)
    {
      availability = f3d::file_availability::SUPPORTED;
      return true;
    }
    else if (stream->Open(fileName.c_str()))
    {
      if (this->canRead(stream))
      {
        availability = f3d::file_availability::SUPPORTED;
        return true;
      }
      else
      {
        availability = f3d::file_availability::UNSUPPORTED_CONTENT;
      }
    }
  }
  return false;
}
} // namespace f3d
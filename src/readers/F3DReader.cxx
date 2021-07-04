#include "F3DReader.h"

#include "F3DReaderFactory.h"

#include <vtksys/SystemTools.hxx>

//----------------------------------------------------------------------------
bool F3DReader::CanRead(const std::string& fileName) const
{
  std::string ext = vtksys::SystemTools::GetFilenameLastExtension(fileName);
  ext = vtksys::SystemTools::LowerCase(ext);

  const std::vector<std::string> extensions = this->GetExtensions();
  for (auto e : extensions)
  {
    if (e == ext)
    {
      return true;
    }
  }
  return false;
}

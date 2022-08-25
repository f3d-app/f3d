#include "F3DReader.h"

#include "F3DReaderFactory.h"

#include <algorithm>
#include <cctype>

//----------------------------------------------------------------------------
bool F3DReader::CanRead(const std::string& fileName) const
{
  std::string ext = fileName.substr(fileName.find_last_of("."));
  std::transform(ext.begin(), ext.end(), ext.begin(), ::tolower);

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

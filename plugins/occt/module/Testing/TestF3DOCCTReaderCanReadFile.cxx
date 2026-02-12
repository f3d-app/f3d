#include "vtkF3DOCCTReader.h"

#include <vtkFileResourceStream.h>
#include <vtkNew.h>

#include <iostream>
#include <optional>

namespace
{
bool TestCanReadFile(
  const std::string& filename, std::optional<vtkF3DOCCTReader::FILE_FORMAT> expectedFormat)
{
  vtkNew<vtkFileResourceStream> stream;
  if (!stream->Open(filename.c_str()))
  {
    std::cerr << "Cannot open file " << filename << "\n";
    return false;
  }

  vtkF3DOCCTReader::FILE_FORMAT format;
  if (!vtkF3DOCCTReader::CanReadFile(stream, format))
  {
    if (expectedFormat.has_value())
    {
      std::cerr << "Unexpected CanReadFile failure for " << filename << "\n";
    }
    return false;
  }

  if (format != expectedFormat.value())
  {
    std::cerr << "Unexpected CanReadFile format for " << filename << "\n";
    return false;
  }

  return true;
}
}

int TestF3DOCCTReaderCanReadFile(int vtkNotUsed(argc), char* argv[])
{
  const std::string data = std::string(argv[1]) + "data";
  bool ret = true;
  ret &= ::TestCanReadFile(data + "/f3d.stp", vtkF3DOCCTReader::FILE_FORMAT::STEP);
  ret &= ::TestCanReadFile(data + "/f3d.igs", vtkF3DOCCTReader::FILE_FORMAT::IGES);
  ret &= ::TestCanReadFile(data + "/f3d.brep", vtkF3DOCCTReader::FILE_FORMAT::BREP);
  ret &= ::TestCanReadFile(data + "/f3d.bin.brep", vtkF3DOCCTReader::FILE_FORMAT::BREP);
#if F3D_PLUGIN_OCCT_XCAF
  ret &= ::TestCanReadFile(data + "/f3d.xbf", vtkF3DOCCTReader::FILE_FORMAT::XBF);
#endif
  ret &= !::TestCanReadFile(data + "/invalid.step", std::nullopt);
  ret &= !::TestCanReadFile(data + "/invalid_endsec.step", std::nullopt);
  ret &= !::TestCanReadFile(data + "/IfcOpenHouse_IFC4.ifc", std::nullopt);
  ret &= !::TestCanReadFile(data + "/invalid.igs", std::nullopt);
  ret &= !::TestCanReadFile(data + "/invalid_second_line.igs", std::nullopt);
  ret &= !::TestCanReadFile(data + "/invalid_second_line.igs", std::nullopt);
  ret &= !::TestCanReadFile(data + "/f3d.vtp", std::nullopt);
  ret &= !vtkF3DOCCTReader::CanReadFile(nullptr);
  return ret ? EXIT_SUCCESS : EXIT_FAILURE;
}

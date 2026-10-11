#include "vtkF3DFCStdReader.h"

#include <vtkFileResourceStream.h>
#include <vtkNew.h>

#include <iostream>

namespace
{
bool TestCanReadFile(const std::string& filename, bool expected)
{
  vtkNew<vtkFileResourceStream> stream;
  if (!stream->Open(filename.c_str()))
  {
    std::cerr << "Cannot open file " << filename << "\n";
    return false;
  }

  if (vtkF3DFCStdReader::CanReadFile(stream) != expected)
  {
    std::cerr << "Unexpected CanReadFile result for " << filename << "\n";
    return false;
  }

  return true;
}
}

int TestF3DFCStdReaderCanReadFile(int vtkNotUsed(argc), char* argv[])
{
  const std::string data = std::string(argv[1]) + "data";
  bool ret = true;
  ret &= TestCanReadFile(data + "/colored_visibility.FCStd", true);
  // not a zip archive
  ret &= TestCanReadFile(data + "/f3d.stp", false);
  // a zip archive but not a FCStd file
  ret &= TestCanReadFile(data + "/cube_gears.3mf", false);
  return ret ? EXIT_SUCCESS : EXIT_FAILURE;
}

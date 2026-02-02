#include "vtkF3DAssimpImporter.h"

#include <vtkDataSet.h>
#include <vtkFileResourceStream.h>
#include <vtkMapper.h>
#include <vtkNew.h>
#include <vtkRenderer.h>
#include <vtkTestUtilities.h>

#include <iostream>

namespace
{
bool TestCanReadFile(const std::string& filename, const std::string& expectedHint)
{
  vtkNew<vtkFileResourceStream> stream;
  if (!stream->Open(filename.c_str()))
  {
    std::cerr << "Cannot open file "<< filename << "\n";
    return false;
  }

  std::string hint;
  if (!vtkF3DAssimpImporter::CanReadFile(stream, hint))
  {
    std::cerr << "Unexpected CanReadFile failure for " << filename << "\n";
    return false;
  }

  if (hint != expectedHint)
  {
    std::cerr << "Unexpected CanReadFile hint: " << hint << " , expecting " << expectedHint << "\n";
    return false;
  }

  return true;
}
}

int TestF3DAssimpImportCanReadFile(int vtkNotUsed(argc), char* argv[])
{
  bool result = true;
  result &= ::TestCanReadFile(std::string(argv[1]) + "data/texturedCube.fbx", "fbx");
  result &= ::TestCanReadFile(std::string(argv[1]) + "data/duck.dae", "dae");
  return result ? EXIT_SUCCESS : EXIT_FAILURE;
}

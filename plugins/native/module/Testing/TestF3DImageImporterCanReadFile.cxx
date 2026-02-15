#include "vtkF3DImageImporter.h"

#include <vtkDataSet.h>
#include <vtkFileResourceStream.h>
#include <vtkMapper.h>
#include <vtkNew.h>
#include <vtkRenderer.h>
#include <vtkTestUtilities.h>

#include <iostream>
#include <string_view>

namespace
{
bool TestCanReadFile(const std::string& filename, std::string_view expectedHint)
{
  vtkNew<vtkFileResourceStream> stream;
  if (!stream->Open(filename.c_str()))
  {
    std::cerr << "Cannot open file " << filename << "\n";
    return false;
  }

  std::string hint;
  if (!vtkF3DImageImporter::CanReadFile(stream, hint))
  {
    if (!expectedHint.empty())
    {
      std::cerr << "Unexpected CanReadFile failure for " << filename << "\n";
    }
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

int TestF3DImageImporterCanReadFile(int vtkNotUsed(argc), char* argv[])
{
  bool result = true;
  result &= ::TestCanReadFile(std::string(argv[1]) + "data/world.png", "png");
  result &= ::TestCanReadFile(std::string(argv[1]) + "data/albedo.bmp", "bmp");
  result &= ::TestCanReadFile(std::string(argv[1]) + "data/viridis32.hdr", "hdr");
  result &= ::TestCanReadFile(std::string(argv[1]) + "data/world.jpg", "jpeg");
  result &= ::TestCanReadFile(std::string(argv[1]) + "data/world.tga", "tga");
#if F3D_MODULE_EXR
  result &= ::TestCanReadFile(std::string(argv[1]) + "data/Rec709.exr", "webp");
#endif
#if F3D_MODULE_WEBP
  result &= ::TestCanReadFile(std::string(argv[1]) + "data/image.webp", "webp");
#endif
  result &= !::TestCanReadFile(std::string(argv[1]) + "data/f3d.vtp", "");
  result &= !::vtkF3DImageImporter::CanReadFile(nullptr);
  return result ? EXIT_SUCCESS : EXIT_FAILURE;
}

#include <vtkFileResourceStream.h>
#include <vtkMultiBlockDataSet.h>
#include <vtkNew.h>
#include <vtkTestUtilities.h>

#include "vtkF3DOCCTReader.h"

#include <iostream>

bool testReaderStream(const std::string& filename, const vtkF3DOCCTReader::FILE_FORMAT& format)
{
  vtkNew<vtkFileResourceStream> fileStream;
  fileStream->Open(filename.c_str());

  if (!vtkF3DOCCTReader::CanReadFile(fileStream))
  {
    return false;
  }

  vtkNew<vtkF3DOCCTReader> reader;
  reader->RelativeDeflectionOn();
  reader->SetLinearDeflection(0.1);
  reader->SetAngularDeflection(0.5);
  reader->ReadWireOn();
  reader->SetStream(fileStream);
  reader->SetFileFormat(format);
  reader->Update();
  return reader->GetOutput()->GetNumberOfPoints() > 0;
}

int TestF3DOCCTReaderStream(int vtkNotUsed(argc), char* argv[])
{
  const std::string data = std::string(argv[1]) + "data";
  bool ret = true;
  ret &= testReaderStream(data + "/f3d.stp", vtkF3DOCCTReader::FILE_FORMAT::STEP);
  // OCCT doesn't support reading IGES stream yet
  // https://dev.opencascade.org/content/reading-iges-stream-seems-broken-770
  //  ret &= testReaderStream(data + "/f3d.igs", vtkF3DOCCTReader::FILE_FORMAT::IGES);
  ret &= testReaderStream(data + "/f3d.brep", vtkF3DOCCTReader::FILE_FORMAT::BREP);
  ret &= testReaderStream(data + "/f3d.bin.brep", vtkF3DOCCTReader::FILE_FORMAT::BREP);
#if F3D_PLUGIN_OCCT_XCAF
  ret &= testReaderStream(data + "/f3d.xbf", vtkF3DOCCTReader::FILE_FORMAT::XBF);
#endif
  return ret ? EXIT_SUCCESS : EXIT_FAILURE;
}

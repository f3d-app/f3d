#include <vtkCallbackCommand.h>
#include <vtkExecutive.h>
#include <vtkFileResourceStream.h>
#include <vtkMultiBlockDataSet.h>
#include <vtkNew.h>
#include <vtkTestUtilities.h>

#include "vtkF3DOCCTReader.h"

#include <iostream>

class ErrorEventCallback : public vtkCommand
{
public:
  static ErrorEventCallback* New()
  {
    return new ErrorEventCallback;
  }

  void Execute(vtkObject* caller, unsigned long vtkNotUsed(evId), void* data) override
  {
    const vtkF3DOCCTReader* reader = reinterpret_cast<vtkF3DOCCTReader*>(caller);
    char* message = static_cast<char*>(data);
    if (reader && message)
    {
      this->Messages.emplace_back(message);
    }
  }

  const std::vector<std::string>& GetRecordedErrorMessages() const
  {
    return this->Messages;
  }

private:
  std::vector<std::string> Messages;
};

bool testReaderStreamError(const std::string& filename, const vtkF3DOCCTReader::FILE_FORMAT& format)
{
  vtkNew<ErrorEventCallback> errorEventCallback;
  vtkNew<vtkCallbackCommand> nullEventCallback;

  vtkNew<vtkFileResourceStream> fileStream;
  fileStream->Open(filename.c_str());

  vtkNew<vtkF3DOCCTReader> reader;
  reader->AddObserver(vtkCommand::ErrorEvent, errorEventCallback);
  reader->GetExecutive()->AddObserver(vtkCommand::ErrorEvent, nullEventCallback);

  reader->RelativeDeflectionOn();
  reader->SetLinearDeflection(0.1);
  reader->SetAngularDeflection(0.5);
  reader->ReadWireOn();
  reader->SetStream(fileStream);
  reader->SetFileFormat(format);
  reader->Update();

  auto errorMessages = errorEventCallback->GetRecordedErrorMessages();
  if (errorMessages.empty())
  {
    std::cerr << "No error triggered.\n";
    return false;
  }

  auto firstMessage = errorMessages.front();
  if (firstMessage.find("Failed to read ") == std::string::npos)
  {
    std::cerr << "No stream error reported ";
    return false;
  }

  return reader->GetOutput()->GetNumberOfPoints() == 0 ? true : false;
}

int TestF3DOCCTReaderStreamError(int vtkNotUsed(argc), char* argv[])
{
  const std::string data = std::string(argv[1]) + "data";
  bool ret = true;
  ret &= testReaderStreamError(data + "/invalid.stp", vtkF3DOCCTReader::FILE_FORMAT::STEP);
  // occt doesn support reader IGES stream yet
  // https://dev.opencascade.org/content/reading-iges-stream-seems-broken-770
  ret &= testReaderStreamError(data + "/f3d.igs", vtkF3DOCCTReader::FILE_FORMAT::IGES);
  ret &= testReaderStreamError(data + "/invalid.brep", vtkF3DOCCTReader::FILE_FORMAT::BREP);
#if F3D_PLUGIN_OCCT_XCAF
  ret &= testReaderStreamError(data + "/invalid.xbf", vtkF3DOCCTReader::FILE_FORMAT::XBF);
#endif
  return ret ? EXIT_SUCCESS : EXIT_FAILURE;
}

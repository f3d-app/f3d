#include <vtkCallbackCommand.h>
#include <vtkExecutive.h>
#include <vtkFileResourceStream.h>
#include <vtkNew.h>
#include <vtkTestUtilities.h>

#include "vtkF3DAlembicReader.h"

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
    const vtkF3DAlembicReader* reader = reinterpret_cast<vtkF3DAlembicReader*>(caller);
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

int TestF3DAlembicReaderStreamError(int vtkNotUsed(argc), char* argv[])
{
  vtkNew<ErrorEventCallback> errorEventCallback;
  vtkNew<vtkCallbackCommand> nullEventCallback;

  std::string filename = std::string(argv[1]) + "data/invalid.abc";

  vtkNew<vtkFileResourceStream> fileStream;
  fileStream->Open(filename.c_str());

  vtkNew<vtkF3DAlembicReader> reader;
  reader->SetStream(fileStream);
  reader->AddObserver(vtkCommand::ErrorEvent, errorEventCallback);
  reader->GetExecutive()->AddObserver(vtkCommand::ErrorEvent, nullEventCallback);

  reader->Update();

  auto errorMessages = errorEventCallback->GetRecordedErrorMessages();
  if (errorMessages.empty())
  {
    std::cerr << "No error triggered.\n";
    return EXIT_FAILURE;
  }

  auto firstMessage = errorMessages.front();
  if (firstMessage.find("Error reading stream") == std::string::npos)
  {
    std::cerr << "No stream error triggered!\n";
    return EXIT_FAILURE;
  }

  return reader->GetOutput()->GetNumberOfPoints() == 0 ? EXIT_SUCCESS : EXIT_FAILURE;
}

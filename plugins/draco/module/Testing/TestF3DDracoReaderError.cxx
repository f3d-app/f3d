#include "vtkF3DDracoReader.h"

#include <vtkCallbackCommand.h>
#include <vtkCommand.h>
#include <vtkExecutive.h>
#include <vtkNew.h>

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
    const vtkF3DDracoReader* importer = reinterpret_cast<vtkF3DDracoReader*>(caller);
    char* message = static_cast<char*>(data);
    if (importer && message)
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

int TestF3DDracoReaderError(int vtkNotUsed(argc), char* argv[])
{
  vtkNew<ErrorEventCallback> errorEventCallback;
  vtkNew<vtkCallbackCommand> nullEventCallback;

  std::string filename = std::string(argv[1]) + "data/nonexistent.drc";
  vtkNew<vtkF3DDracoReader> reader;
  reader->AddObserver(vtkCommand::ErrorEvent, errorEventCallback);
  reader->GetExecutive()->AddObserver(vtkCommand::ErrorEvent, nullEventCallback);
  reader->SetFileName(filename);
  reader->Update();

  auto errorMessages = errorEventCallback->GetRecordedErrorMessages();
  if (errorMessages.empty())
  {
    std::cerr << "No error triggered.\n";
    return EXIT_FAILURE;
  }

  auto lastMessage = errorMessages.back();
  if (lastMessage.find("Cannot read file") == std::string::npos)
  {
    std::cerr << "No draco error triggered!\n";
    return EXIT_FAILURE;
  }

  return EXIT_SUCCESS;
}

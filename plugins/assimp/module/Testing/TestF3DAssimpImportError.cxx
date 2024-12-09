#include <vtkNew.h>
#include <vtkTestUtilities.h>

#include "vtkCommand.h"
#include "vtkF3DAssimpImporter.h"

#include <iostream>
#include <vector>

class ErrorEventCallback : public vtkCommand
{
public:
  static ErrorEventCallback* New()
  {
    return new ErrorEventCallback;
  }

  void Execute(vtkObject* caller, unsigned long vtkNotUsed(evId), void* data) override
  {
    auto importer = reinterpret_cast<vtkF3DAssimpImporter*>(caller);
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

int TestF3DAssimpImportError(int vtkNotUsed(argc), char* vtkNotUsed(argv)[])
{
  vtkNew<vtkF3DAssimpImporter> importer;

  vtkNew<ErrorEventCallback> errorEventCallback;
  importer->AddObserver(vtkCommand::ErrorEvent, errorEventCallback);

  importer->SetFileName("dummy.dae");
  importer->Update();

  auto errorMessages = errorEventCallback->GetRecordedErrorMessages();
  if (errorMessages.empty())
  {
    std::cerr << "No error triggered." << std::endl;
    return EXIT_FAILURE;
  }

  auto lastMessage = errorMessages.back();
  if (lastMessage.find("Assimp error") == std::string::npos)
  {
    std::cerr << "No Assimp error triggered!" << std::endl;
    return EXIT_FAILURE;
  }

  if (importer->GetNumberOfAnimations() != 0)
  {
    std::cerr << "Importer has " << importer->GetNumberOfAnimations()
              << " animations, expected 0 animation." << std::endl;
    return EXIT_FAILURE;
  }

  if (!importer->UpdateAtTimeValue(0))
  {
    std::cerr << "Importer did not return true with no animation enabled "
              << "when calling UpdateAtTimeValue()" << std::endl;
    return EXIT_FAILURE;
  }

  return EXIT_SUCCESS;
}

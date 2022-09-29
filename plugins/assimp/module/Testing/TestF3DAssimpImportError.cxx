#include <vtkNew.h>
#include <vtkTestUtilities.h>

#include "vtkCommand.h"
#include "vtkF3DAssimpImporter.h"

#include <iostream>
#include <vector>

class WarningEventCallback : public vtkCommand
{
public:
  static WarningEventCallback* New() { return new WarningEventCallback; }

  void Execute(vtkObject* caller, unsigned long evId, void* data) override
  {
    auto importer = reinterpret_cast<vtkF3DAssimpImporter*>(caller);
    char* message = static_cast<char*>(data);
    if (importer && message)
    {
      this->Messages.push_back(message);
    }
  }

  const std::vector<std::string>& GetRecordedWarningMessages() const { return this->Messages; }

private:
  std::vector<std::string> Messages;
};

int TestF3DAssimpImportError(int argc, char* argv[])
{
  vtkObject::GlobalWarningDisplayOn();
  vtkNew<vtkF3DAssimpImporter> importer;

  vtkNew<WarningEventCallback> warningEventCallback;
  importer->AddObserver(vtkCommand::WarningEvent, warningEventCallback);

  importer->SetFileName("dummy.dae");
  importer->Update();

  auto warningMessages = warningEventCallback->GetRecordedWarningMessages();
  if (warningMessages.empty())
  {
    std::cerr << "No warning triggered." << std::endl;
    return EXIT_FAILURE;
  }

  auto lastMessage = warningMessages.back();
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

  return EXIT_SUCCESS;
}

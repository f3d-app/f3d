#include <vtkNew.h>
#include <vtkObjectFactory.h>

#include "vtkF3DUIActor.h"

#include <iostream>

// subclass to retrieve internal protected values
class vtkF3DTestUIActor : public vtkF3DUIActor
{
public:
  static vtkF3DTestUIActor* New();
  vtkTypeMacro(vtkF3DTestUIActor, vtkF3DUIActor);

  size_t GetNumberOfFrameTimes()
  {
    return this->FrameTimes.size();
  }
  double GetTotalFrameTimes()
  {
    return this->TotalFrameTimes;
  }
  int GetFpsValue()
  {
    return this->FpsValue;
  }
};

vtkObjectFactoryNewMacro(vtkF3DTestUIActor);

int TestF3DFpsCounter(int argc, char* argv[])
{
  vtkNew<vtkF3DTestUIActor> uiActor;

  // add 1000 frames at 0.01s
  for (int i = 0; i < 1000; i++)
  {
    uiActor->UpdateFpsValue(0.01);
  }

  // make sure only the 100 last records are kept
  if (uiActor->GetNumberOfFrameTimes() > 100)
  {
    std::cerr << "Number of frame times must be at most 100\n";
    return EXIT_FAILURE;
  }

  // make sure only the total time kept doesn't exceed 1 second
  if (uiActor->GetTotalFrameTimes() > 1.0)
  {
    std::cerr << "Number of total frame times must be at most 1.0\n";
    return EXIT_FAILURE;
  }

  // make sure the FPS value is exactly 100
  if (uiActor->GetFpsValue() != 100)
  {
    std::cerr << "Number of FPS value must be exactly 100\n";
    return EXIT_FAILURE;
  }

  return EXIT_SUCCESS;
}

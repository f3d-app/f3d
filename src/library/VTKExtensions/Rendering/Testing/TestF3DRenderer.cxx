#include <vtkActor.h>
#include <vtkCamera.h>
#include <vtkNew.h>
#include <vtkPolyData.h>
#include <vtkPolyDataMapper.h>
#include <vtkRenderWindow.h>
#include <vtkRenderWindowInteractor.h>
#include <vtkXMLPolyDataReader.h>

#include "F3DOffscreenRender.h"
#include "vtkRenderer.h"

//------------------------------------------------------------------------------
int TestF3DRenderer(int argc, char* argv[])
{
  //  TODO actually test vtkF3DRenderer
  std::string filename = std::string(argv[1]) + "data/cow.vtp";

  // Read all the data from the file
  vtkNew<vtkXMLPolyDataReader> reader;
  reader->SetFileName(filename.c_str());
  reader->Update();

  // Visualize
  vtkNew<vtkPolyDataMapper> mapper;
  mapper->SetInputConnection(reader->GetOutputPort());

  vtkNew<vtkActor> actor;
  actor->SetMapper(mapper);

  vtkNew<vtkRenderer> renderer;
  vtkNew<vtkRenderWindow> renderWindow;
  renderWindow->SetOffScreenRendering(true);
  renderWindow->AddRenderer(renderer);
  vtkNew<vtkRenderWindowInteractor> renderWindowInteractor;
  renderWindowInteractor->SetRenderWindow(renderWindow);

  renderer->AddActor(actor);
  renderer->ResetCamera();

  renderWindow->SetSize(300, 300);
  renderWindow->Render();

  std::string baseline = std::string(argv[0]) + ".png";
  std::string basefileName = std::string(argv[1]) + "baselines/" + baseline;
  std::string outputFileName = std::string(argv[2]) + baseline;
  return F3DOffscreenRender::RenderTesting(renderWindow, basefileName, 50, false, outputFileName)
    ? EXIT_SUCCESS
    : EXIT_FAILURE;
}

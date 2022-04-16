#include "F3DOffscreenRender.h"

#include <vtkActor.h>
#include <vtkDataSetMapper.h>
#include <vtkImageData.h>
#include <vtkNew.h>
#include <vtkPolyDataMapper.h>
#include <vtkRenderWindow.h>
#include <vtkRenderWindowInteractor.h>
#include <vtkRenderer.h>
#include <vtkTestUtilities.h>
#include <vtkXMLPolyDataReader.h>

#include <iostream>

int constexpr DIFF_THRESHOLD = 100;

int TestF3DOffscreenRender(int argc, char* argv[])
{
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

  // Test RenderTesting
  std::string baseline = std::string(argv[0]) + ".png";
  std::string baselinePath = std::string(argv[1]) + "baselines/" + baseline;
  std::string outputPath = std::string(argv[2]) + baseline;
  if (!F3DOffscreenRender::RenderTesting(
        renderWindow, baselinePath, DIFF_THRESHOLD, false, outputPath))
  {
    std::cerr << "RenderTesting failed" << std::endl;
    return EXIT_FAILURE;
  }

  // Test RenderTesting no background
  std::string baselineNoBG = std::string(argv[0]) + "NoBG.png";
  std::string baselineNoBGPath = std::string(argv[1]) + "baselines/" + baselineNoBG;
  std::string outputNoBGPath = std::string(argv[2]) + baselineNoBG;
  if (!F3DOffscreenRender::RenderTesting(
        renderWindow, baselineNoBGPath, DIFF_THRESHOLD, true, outputNoBGPath))
  {
    std::cerr << "RenderTesting no background failed" << std::endl;
    return EXIT_FAILURE;
  }

  // Test RenderOffscreen
  std::string tmpOutputPath = std::string(argv[2]) + "tmp" + baseline;
  std::string tmp2OutputPath = std::string(argv[2]) + "tmp2" + baseline;
  if (!F3DOffscreenRender::RenderOffScreen(renderWindow, tmpOutputPath, false))
  {
    std::cerr << "RenderOffscreen failed" << std::endl;
    return EXIT_FAILURE;
  }
  if (!F3DOffscreenRender::RenderTesting(
        renderWindow, tmpOutputPath, DIFF_THRESHOLD, false, tmp2OutputPath))
  {
    std::cerr << "RenderTesting on RenderOffscreen result failed" << std::endl;
    return EXIT_FAILURE;
  }

  // Test RenderOffscreen no background
  if (!F3DOffscreenRender::RenderOffScreen(renderWindow, tmpOutputPath, true))
  {
    std::cerr << "RenderOffScreen no background failed" << std::endl;
    return EXIT_FAILURE;
  }
  if (!F3DOffscreenRender::RenderTesting(
        renderWindow, tmpOutputPath, DIFF_THRESHOLD, true, tmp2OutputPath))
  {
    std::cerr << "RenderTesting on RenderOffscreen result with no background failed" << std::endl;
    return EXIT_FAILURE;
  }

  // Test RenderToImage
  vtkNew<vtkImageData> img;
  if (!F3DOffscreenRender::RenderToImage(renderWindow, img, false))
  {
    std::cerr << "RenderToImage failed" << std::endl;
    return EXIT_FAILURE;
  }

  vtkNew<vtkDataSetMapper> imgMapper;
  imgMapper->SetInputData(img);

  vtkNew<vtkActor> imgActor;
  imgActor->SetMapper(imgMapper);

  vtkNew<vtkRenderer> imgRenderer;
  vtkNew<vtkRenderWindow> imgRenderWindow;
  imgRenderWindow->SetOffScreenRendering(true);
  imgRenderWindow->AddRenderer(imgRenderer);
  vtkNew<vtkRenderWindowInteractor> imgRenderWindowInteractor;
  imgRenderWindowInteractor->SetRenderWindow(imgRenderWindow);

  imgRenderer->SetBackground(1., 1., 1.);
  imgRenderer->AddActor(imgActor);
  imgRenderer->ResetCamera();

  imgRenderWindow->SetSize(300, 300);
  imgRenderWindow->Render();

  std::string baselineImg = std::string(argv[0]) + "Img.png";
  std::string baselineImgPath = std::string(argv[1]) + "baselines/" + baselineImg;
  std::string outputImgPath = std::string(argv[2]) + baselineImg;
  if (!F3DOffscreenRender::RenderTesting(
        imgRenderWindow, baselineImgPath, DIFF_THRESHOLD, false, outputImgPath))
  {
    std::cerr << "RenderTesting on RenderToImage result failed" << std::endl;
    return EXIT_FAILURE;
  }

  // Test RenderToImage no background
  if (!F3DOffscreenRender::RenderToImage(renderWindow, img, true))
  {
    std::cerr << "RenderToImage no background failed" << std::endl;
    return EXIT_FAILURE;
  }
  imgRenderWindow->Render();

  std::string baselineImgNoBG = std::string(argv[0]) + "ImgNoBG.png";
  std::string baselineImgNoBGPath = std::string(argv[1]) + "baselines/" + baselineImgNoBG;
  std::string outputImgNoBGPath = std::string(argv[2]) + baselineImgNoBG;
  if (!F3DOffscreenRender::RenderTesting(
        imgRenderWindow, baselineImgNoBGPath, DIFF_THRESHOLD, false, outputImgNoBGPath))
  {
    std::cerr << "RenderTesting on RenderToImage result with no background failed" << std::endl;
    return EXIT_FAILURE;
  }

  return EXIT_SUCCESS;
}

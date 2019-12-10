#include "F3DOffscreenRender.h"

#include <vtkImageDifference.h>
#include <vtkNew.h>
#include <vtkPNGReader.h>
#include <vtkPNGWriter.h>
#include <vtkRenderWindow.h>
#include <vtkWindowToImageFilter.h>
#include <vtksys/SystemTools.hxx>

int F3DOffscreenRender::RenderOffScreen(vtkRenderWindow* renWin, const std::string& output)
{
  renWin->OffScreenRenderingOn();

  vtkNew<vtkWindowToImageFilter> rtW2if;
  rtW2if->SetInput(renWin);

  vtkNew<vtkPNGWriter> writer;
  writer->SetInputConnection(rtW2if->GetOutputPort());
  std::string fullPath = vtksys::SystemTools::CollapseFullPath(output);
  writer->SetFileName(fullPath.c_str());
  writer->Write();

  return EXIT_SUCCESS;
}

int F3DOffscreenRender::RenderTesting(
  vtkRenderWindow* renWin, const std::string& reference, double threshold)
{
  renWin->OffScreenRenderingOn();

  vtkNew<vtkWindowToImageFilter> rtW2if;
  rtW2if->SetInput(renWin);

  // Disable hardware antialiasing
  renWin->SetMultiSamples(0);

  if (vtksys::SystemTools::HasEnv("F3D_GEN_REF"))
  {
    vtkNew<vtkPNGWriter> writer;
    writer->SetInputConnection(rtW2if->GetOutputPort());
    std::string fullPath = vtksys::SystemTools::CollapseFullPath(reference);
    writer->SetFileName(fullPath.c_str());
    writer->Write();

    cerr << "Reference generated!" << endl;

    return EXIT_SUCCESS;
  }

  if (!vtksys::SystemTools::FileExists(reference))
  {
    cerr << "Reference file does not exists, "
         << "generate it first (set F3D_GEN_REF variable and run test again)." << endl;

    return EXIT_FAILURE;
  }

  vtkNew<vtkPNGReader> reader;
  reader->SetFileName(reference.c_str());

  vtkNew<vtkImageDifference> diff;
  diff->SetInputConnection(rtW2if->GetOutputPort());
  diff->SetImageConnection(reader->GetOutputPort());
  diff->Update();

  double error = diff->GetThresholdedError();
  cout << "Diff threshold error = " << error << endl;
  if (error > threshold)
  {
    std::string fileName = vtksys::SystemTools::GetFilenameName(reference.c_str());
    std::string testFileName = vtksys::SystemTools::CollapseFullPath(fileName);

    std::string modFileName = fileName;
    vtksys::SystemTools::ReplaceString(modFileName, ".png", ".diff.png");
    std::string diffFileName = vtksys::SystemTools::CollapseFullPath(modFileName);

    vtkNew<vtkPNGWriter> writer;
    writer->SetInputConnection(rtW2if->GetOutputPort());
    writer->SetFileName(fileName.c_str());
    writer->Write();

    writer->SetInputConnection(diff->GetOutputPort());
    writer->SetFileName(diffFileName.c_str());
    writer->Write();

    return EXIT_FAILURE;
  }
  return EXIT_SUCCESS;
}

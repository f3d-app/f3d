#include "F3DOffscreenRender.h"

#include <vtkImageDifference.h>
#include <vtkNew.h>
#include <vtkPNGReader.h>
#include <vtkPNGWriter.h>
#include <vtkRenderWindow.h>
#include <vtkWindowToImageFilter.h>
#include <vtksys/SystemTools.hxx>

#include "F3DLog.h"

//----------------------------------------------------------------------------
int F3DOffscreenRender::RenderOffScreen(
  vtkRenderWindow* renWin, const std::string& output, bool noBg)
{
  vtkNew<vtkWindowToImageFilter> rtW2if;
  rtW2if->SetInput(renWin);

  if (noBg)
  {
    rtW2if->SetInputBufferTypeToRGBA();
  }

  vtkNew<vtkPNGWriter> writer;
  writer->SetInputConnection(rtW2if->GetOutputPort());
  std::string fullPath = vtksys::SystemTools::CollapseFullPath(output);
  writer->SetFileName(fullPath.c_str());
  writer->Write();

  return EXIT_SUCCESS;
}

//----------------------------------------------------------------------------
int F3DOffscreenRender::RenderTesting(vtkRenderWindow* renWin, const std::string& reference,
  double threshold, const std::string& output)
{
  if (!vtksys::SystemTools::FileExists(reference))
  {
    if (output.empty())
    {
      F3DLog::Print(F3DLog::Severity::Error,
        "Reference image does not exists, use the --output option to output current rendering into "
        "an image file.");
    }
    else
    {
      F3DOffscreenRender::RenderOffScreen(renWin, output);
      F3DLog::Print(F3DLog::Severity::Error,
        "Reference file does not exists, current rendering has been outputted to ", output, ".");
    }
    return EXIT_FAILURE;
  }
  vtkNew<vtkWindowToImageFilter> rtW2if;
  rtW2if->SetInput(renWin);

  vtkNew<vtkPNGReader> reader;
  reader->SetFileName(reference.c_str());

  vtkNew<vtkImageDifference> diff;
  diff->SetInputConnection(rtW2if->GetOutputPort());
  diff->SetImageConnection(reader->GetOutputPort());
  diff->Update();

  double error = diff->GetThresholdedError();
  F3DLog::Print(F3DLog::Severity::Info, "Diff threshold error = ", error);
  if (error > threshold)
  {
    F3DLog::Print(F3DLog::Severity::Error,
      "Current rendering difference with reference image is higher than the threshold of ",
      threshold, ".");
    if (output.empty())
    {
      F3DLog::Print(F3DLog::Severity::Error,
        "Use the --output option to be able to output current rendering and diff images into "
        "files.");
    }
    else
    {
      std::string diffFileName = output.c_str();
      vtksys::SystemTools::ReplaceString(diffFileName, ".png", ".diff.png");

      vtkNew<vtkPNGWriter> writer;
      writer->SetInputConnection(rtW2if->GetOutputPort());
      writer->SetFileName(output.c_str());
      writer->Write();

      writer->SetInputConnection(diff->GetOutputPort());
      writer->SetFileName(diffFileName.c_str());
      writer->Write();
      F3DLog::Print(F3DLog::Severity::Error,
        "Current rendering and diff images have been outputted in ", output, " and ", diffFileName);
    }
    return EXIT_FAILURE;
  }
  return EXIT_SUCCESS;
}

#include "F3DOffscreenRender.h"

#include "F3DLog.h"

#include <vtkImageData.h>
#include <vtkImageDifference.h>
#include <vtkNew.h>
#include <vtkPNGReader.h>
#include <vtkPNGWriter.h>
#include <vtkRenderWindow.h>
#include <vtkRenderer.h>
#include <vtkRendererCollection.h>
#include <vtkVersion.h>
#include <vtkWindowToImageFilter.h>
#include <vtksys/SystemTools.hxx>

//----------------------------------------------------------------------------
bool F3DOffscreenRender::RenderToImage(vtkRenderWindow* renWin, vtkImageData* image, bool noBg)
{
  vtkNew<vtkWindowToImageFilter> rtW2if;
  rtW2if->SetInput(renWin);

  if (noBg)
  {
    // we need to set the background to black to avoid blending issues with translucent
    // objects when saving to file with no background
    renWin->GetRenderers()->GetFirstRenderer()->SetBackground(0, 0, 0);
    rtW2if->SetInputBufferTypeToRGBA();
  }

  rtW2if->Update();
  vtkImageData* outImg = rtW2if->GetOutput();
  image->ShallowCopy(outImg);

  return true;
}

//----------------------------------------------------------------------------
bool F3DOffscreenRender::RenderOffScreen(
  vtkRenderWindow* renWin, const std::string& output, bool noBg)
{
  vtkNew<vtkImageData> image;
  F3DOffscreenRender::RenderToImage(renWin, image, noBg);

  vtkNew<vtkPNGWriter> writer;
  writer->SetInputData(image);
  writer->SetFileName(vtksys::SystemTools::CollapseFullPath(output).c_str());
  writer->Write();

  return true;
}

//----------------------------------------------------------------------------
bool F3DOffscreenRender::RenderTesting(vtkRenderWindow* renWin, const std::string& reference,
  double threshold, bool noBg, const std::string& output)
{
  std::string referenceFullPath = vtksys::SystemTools::CollapseFullPath(reference);
  if (!vtksys::SystemTools::FileExists(referenceFullPath))
  {
    if (output.empty())
    {
      F3DLog::Print(F3DLog::Severity::Error,
        "Reference image " + reference + " does not exists, use the output option to output current rendering into "
        "an image file.\n");
    }
    else
    {
#if VTK_VERSION_NUMBER >= VTK_VERSION_CHECK(9, 1, 20211007)
      F3DOffscreenRender::RenderOffScreen(renWin, output, noBg);
#else
      F3DOffscreenRender::RenderOffScreen(renWin, output);
#endif
      F3DLog::Print(F3DLog::Severity::Error,
        "Reference image " + reference + " does not exists, current rendering has been outputted to " + output + ".\n");
    }
    return false;
  }

  vtkNew<vtkWindowToImageFilter> rtW2if;
  rtW2if->SetInput(renWin);
#if VTK_VERSION_NUMBER >= VTK_VERSION_CHECK(9, 1, 20211007)
  if (noBg)
  {
    // we need to set the background to black to avoid blending issues with translucent
    // objects when saving to file with no background
    renWin->GetRenderers()->GetFirstRenderer()->SetBackground(0, 0, 0);
    rtW2if->SetInputBufferTypeToRGBA();
  }
  else
  {
    rtW2if->SetInputBufferTypeToRGB();
  }
#else
  static_cast<void>(noBg);
  rtW2if->SetInputBufferTypeToRGB();
#endif

  vtkNew<vtkPNGReader> reader;
  reader->SetFileName(referenceFullPath.c_str());

  vtkNew<vtkImageDifference> diff;
  // handle threshold outside of vtkImageDifference:
  // https://gitlab.kitware.com/vtk/vtk/-/issues/18152
  diff->SetThreshold(0);
  diff->SetInputConnection(rtW2if->GetOutputPort());
  diff->SetImageConnection(reader->GetOutputPort());
  diff->UpdateInformation();
  double error = diff->GetThresholdedError();

  if (error <= threshold)
  {
    diff->Update();
    error = diff->GetThresholdedError();
  }

  F3DLog::Print(F3DLog::Severity::Info, "Diff threshold error = " + std::to_string(error) + "\n");
  if (error > threshold)
  {
    F3DLog::Print(F3DLog::Severity::Error,
      "Current rendering difference with reference image is higher than the threshold of " +
        std::to_string(threshold) + ".\n");
    if (output.empty())
    {
      F3DLog::Print(F3DLog::Severity::Error,
        "Use the --output option to be able to output current rendering and diff images into "
        "files.\n");
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
        "Current rendering and diff images have been outputted in " + output + " and " +
          diffFileName + "\n");
    }
    return false;
  }
  return true;
}

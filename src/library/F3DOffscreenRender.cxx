#include "F3DOffscreenRender.h"

#include "f3d_log.h"

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
      f3d::log::error(
        "Reference image does not exists, use the --output option to output current rendering into "
        "an image file.");
    }
    else
    {
#if VTK_VERSION_NUMBER >= VTK_VERSION_CHECK(9, 1, 20211007)
      F3DOffscreenRender::RenderOffScreen(renWin, output, noBg);
#else
      F3DOffscreenRender::RenderOffScreen(renWin, output);
#endif
      f3d::log::error(
        "Reference file does not exists, current rendering has been outputted to ", output, ".");
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
  diff->SetInputConnection(rtW2if->GetOutputPort());
  diff->SetImageConnection(reader->GetOutputPort());
  diff->Update();

  double error = diff->GetThresholdedError();
  f3d::log::info("Diff threshold error = ", error);
  if (error > threshold)
  {
    f3d::log::error(
      "Current rendering difference with reference image is higher than the threshold of ",
      threshold, ".");
    if (output.empty())
    {
      f3d::log::error(
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
      f3d::log::error(
        "Current rendering and diff images have been outputted in ", output, " and ", diffFileName);
    }
    return false;
  }
  return true;
}

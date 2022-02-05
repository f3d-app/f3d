#include "f3d_windowStandard.h"

#include "F3DLog.h"
#include "f3d_options.h"
#include "vtkF3DRendererWithColoring.h"

#include <vtkCamera.h>
#include <vtkPNGReader.h>
#include <vtkRenderWindow.h>
#include <vtkVersion.h>

namespace
{
void DisplayCameraInformation(vtkCamera* cam)
{
  double* position = cam->GetPosition();
  F3DLog::Print(F3DLog::Severity::Info, "Camera position is: ", position[0], ", ", position[1],
    ", ", position[2], ".");
  double* focalPoint = cam->GetFocalPoint();
  F3DLog::Print(F3DLog::Severity::Info, "Camera focal point is: ", focalPoint[0], ", ",
    focalPoint[1], ", ", focalPoint[2], ".");
  double* viewUp = cam->GetViewUp();
  F3DLog::Print(F3DLog::Severity::Info, "Camera view up is: ", viewUp[0], ", ", viewUp[1], ", ",
    viewUp[2], ".");
  F3DLog::Print(F3DLog::Severity::Info, "Camera view angle is: ", cam->GetViewAngle(), ".\n");
}
}

namespace f3d
{
class windowStandard::F3DInternals
{
public:
  void UpdateCamera(const f3d::options* options)
  {
    if (this->Renderer)
    {
      // Set the initial camera once all options
      // have been shown as they may have an effect on it
      if (options->get<int>("camera-index") < 0)
      {
        // set a default camera from bounds using VTK method
        this->Renderer->vtkRenderer::ResetCamera();

        // use options to overwrite camera parameters
        vtkCamera* cam = this->Renderer->GetActiveCamera();

        std::vector<double> cameraPosition = options->get<std::vector<double> >("camera-position");
        if (cameraPosition.size() == 3)
        {
          cam->SetPosition(cameraPosition.data());
        }

        std::vector<double> cameraFocalPoint =
          options->get<std::vector<double> >("camera-focal-point");
        if (cameraFocalPoint.size() == 3)
        {
          cam->SetFocalPoint(cameraFocalPoint.data());
        }

        std::vector<double> cameraViewUp = options->get<std::vector<double> >("camera-view-up");
        if (cameraViewUp.size() == 3)
        {
          cam->SetViewUp(cameraViewUp.data());
        }

        double cameraViewAngle = options->get<double>("camera-view-angle");
        if (cameraViewAngle != 0)
        {
          cam->SetViewAngle(cameraViewAngle);
        }

        cam->Azimuth(options->get<double>("camera-azimuth-angle"));
        cam->Elevation(options->get<double>("camera-elevation-angle"));
        cam->OrthogonalizeViewUp();

        if (options->get<bool>("verbose"))
        {
          ::DisplayCameraInformation(cam);
        }
      }

      this->Renderer->InitializeCamera();
    }
  }

  vtkNew<vtkRenderWindow> RenWin;
  vtkSmartPointer<vtkF3DRenderer> Renderer;
};

//----------------------------------------------------------------------------
windowStandard::windowStandard(
  const std::string& windowName, bool offscreen, const void* icon, size_t iconSize)
  : Internals(new windowStandard::F3DInternals)
{
  this->Internals->RenWin->SetMultiSamples(0); // Disable hardware antialiasing
  this->Internals->RenWin->SetWindowName(windowName.c_str());

  // Offscreen rendering must be set before initializing interactor
  this->Internals->RenWin->SetOffScreenRendering(offscreen);

#if VTK_VERSION_NUMBER >= VTK_VERSION_CHECK(9, 0, 20200615)
  // set icon
  vtkNew<vtkPNGReader> iconReader;
  iconReader->SetMemoryBuffer(icon);
  iconReader->SetMemoryBufferLength(iconSize);
  iconReader->Update();
  // TODO
//  this->Internals->RenWin->SetIcon(iconReader->GetOutput());
#else
  (void)icon;
  (void)iconSize;
#endif
}

//----------------------------------------------------------------------------
windowStandard::~windowStandard() = default;

//----------------------------------------------------------------------------
void windowStandard::Initialize(bool withColoring, std::string fileInfo)
{
  // Clear renderer if already present
  if (this->Internals->Renderer)
  {
    this->Internals->Renderer->ShowAxis(false);
    this->Internals->RenWin->RemoveRenderer(this->Internals->Renderer);
  }

  // TODO test should not be needed
  if (this->Internals->RenWin)
  {
    this->Internals->RenWin->SetSize(this->Options->get<std::vector<int> >("resolution").data());
    this->Internals->RenWin->SetFullScreen(this->Options->get<bool>("fullscreen"));

    if (withColoring)
    {
      this->Internals->Renderer = vtkSmartPointer<vtkF3DRendererWithColoring>::New();
    }
    else
    {
      this->Internals->Renderer = vtkSmartPointer<vtkF3DRenderer>::New();
    }
    this->Internals->RenWin->AddRenderer(this->Internals->Renderer);
    this->Internals->Renderer->Initialize(*this->Options, fileInfo);
  }
}

//----------------------------------------------------------------------------
void windowStandard::update()
{
  if (this->Internals->Renderer)
  {
    // Make sure options are up to date
    this->Internals->Renderer->UpdateOptions(*this->Options);

    // Actors are loaded, use the bounds to reset camera and set-up SSAO
    this->Internals->Renderer->ShowOptions();
    this->Internals->UpdateCamera(this->Options);
  }
}

//----------------------------------------------------------------------------
vtkRenderWindow* windowStandard::GetRenderWindow()
{
  return this->Internals->RenWin;
}

//----------------------------------------------------------------------------
vtkF3DRenderer* windowStandard::GetRenderer()
{
  return this->Internals->Renderer;
}
};

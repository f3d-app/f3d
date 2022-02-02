#include "f3d_window.h"

#include "vtkF3DRendererWithColoring.h"
#include "F3DLog.h"
#include "f3d_options.h"
#include "f3d_loader.h"

#include <vtkVersion.h>
#include <vtkRenderWindow.h>
#include <vtkCamera.h>
#include <vtkPNGReader.h>

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
class window::F3DInternals
{
public:
  F3DInternals()
  {
  }

  void UpdateCamera()
  {
    if (this->Renderer)
    {
      // Set the initial camera once all options
      // have been shown as they may have an effect on it
      if (this->Options->get<int>("camera-index") < 0)
      {
        // set a default camera from bounds using VTK method
        this->Renderer->vtkRenderer::ResetCamera();

        // use options to overwrite camera parameters
        vtkCamera* cam = this->Renderer->GetActiveCamera();

        std::vector<double> cameraPosition =
          this->Options->get<std::vector<double> >("camera-position");
        if (cameraPosition.size() == 3)
        {
          cam->SetPosition(cameraPosition.data());
        }

        std::vector<double> cameraFocalPoint =
          this->Options->get<std::vector<double> >("camera-focal-point");
        if (cameraFocalPoint.size() == 3)
        {
          cam->SetFocalPoint(cameraFocalPoint.data());
        }

        std::vector<double> cameraViewUp =
          this->Options->get<std::vector<double> >("camera-view-up");
        if (cameraViewUp.size() == 3)
        {
          cam->SetViewUp(cameraViewUp.data());
        }

        double cameraViewAngle = this->Options->get<double>("camera-view-angle");
        if (cameraViewAngle != 0)
        {
          cam->SetViewAngle(cameraViewAngle);
        }

        cam->Azimuth(this->Options->get<double>("camera-azimuth-angle"));
        cam->Elevation(this->Options->get<double>("camera-elevation-angle"));
        cam->OrthogonalizeViewUp();

        if (this->Options->get<bool>("verbose"))
        {
          ::DisplayCameraInformation(cam);
        }
      }

      this->Renderer->InitializeCamera();
    }
  }

  vtkSmartPointer<vtkRenderWindow> RenWin;
  vtkSmartPointer<vtkF3DRenderer> Renderer;
  f3d::loader* Loader;
  const f3d::options* Options;
};

//----------------------------------------------------------------------------
window::window(const std::string& windowName, bool offscreen, const void* icon, size_t iconSize)
  : Internals(new window::F3DInternals)
{
  // TODO Should be vtkNew unless we can create other type of windows
  this->Internals->RenWin = vtkSmartPointer<vtkRenderWindow>::New();
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
window::~window() = default;

//----------------------------------------------------------------------------
void window::SetLoader(loader* loader)
{
  this->Internals->Loader = loader;
  this->Internals->Options = &loader->getOptions();
}
  
//----------------------------------------------------------------------------
void window::Initialize(bool withColoring, std::string fileInfo)
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
    this->Internals->RenWin->SetSize(
      this->Internals->Options->get<std::vector<int> >("resolution").data());
    this->Internals->RenWin->SetFullScreen(this->Internals->Options->get<bool>("fullscreen"));

    if (withColoring)
    {
      this->Internals->Renderer = vtkSmartPointer<vtkF3DRendererWithColoring>::New();
    }
    else
    {
      this->Internals->Renderer = vtkSmartPointer<vtkF3DRenderer>::New();
    }
    this->Internals->RenWin->AddRenderer(this->Internals->Renderer);
    this->Internals->Renderer->Initialize(*this->Internals->Options, fileInfo);
  }
}

//----------------------------------------------------------------------------
void window::update()
{
  if (this->Internals->Renderer)
  {
    // Make sure options are up to date
    this->Internals->Renderer->UpdateOptions(*this->Internals->Options);
    
    // Actors are loaded, use the bounds to reset camera and set-up SSAO
    this->Internals->UpdateCamera();
    this->Internals->Renderer->ShowOptions();
  }
}

//----------------------------------------------------------------------------
vtkRenderWindow* window::GetRenderWindow()
{
  return this->Internals->RenWin;
}

//----------------------------------------------------------------------------
vtkF3DRenderer* window::GetRenderer()
{
  return this->Internals->Renderer;
}
};

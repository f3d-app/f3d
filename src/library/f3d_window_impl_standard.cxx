#include "f3d_window_impl_standard.h"

#include "F3DOffscreenRender.h"

#include "f3d_log.h"
#include "f3d_options.h"
#include "vtkF3DConfigure.h"
#include "vtkF3DGenericImporter.h"
#include "vtkF3DRendererWithColoring.h"

#include <vtkCamera.h>
#include <vtkPNGReader.h>
#include <vtkPointGaussianMapper.h>
#include <vtkRenderWindow.h>
#include <vtkVersion.h>

#if F3D_HAS_EGL_SUPPORT
#include <vtkEGLRenderWindow.h>
#endif

namespace
{
void DisplayCameraInformation(vtkCamera* cam)
{
  double* position = cam->GetPosition();
  f3d::log::info("Camera position is: ", position[0], ", ", position[1], ", ", position[2], ".");
  double* focalPoint = cam->GetFocalPoint();
  f3d::log::info(
    "Camera focal point is: ", focalPoint[0], ", ", focalPoint[1], ", ", focalPoint[2], ".");
  double* viewUp = cam->GetViewUp();
  f3d::log::info("Camera view up is: ", viewUp[0], ", ", viewUp[1], ", ", viewUp[2], ".");
  f3d::log::info("Camera view angle is: ", cam->GetViewAngle(), ".\n");
}
}

namespace f3d
{
class window_impl_standard::F3DInternals
{
public:
  bool UpdateCamera(const f3d::options& options)
  {
    if (!this->Renderer)
    {
      return false;
    }

    // Set the initial camera once all options
    // have been shown as they may have an effect on it
    if (options.getAsInt("camera-index") < 0)
    {
      // set a default camera from bounds using VTK method
      this->Renderer->vtkRenderer::ResetCamera();

      // use options to overwrite camera parameters
      vtkCamera* cam = this->Renderer->GetActiveCamera();

      std::vector<double> cameraPosition = options.getAsDoubleVector("camera-position");
      if (cameraPosition.size() == 3)
      {
        cam->SetPosition(cameraPosition.data());
      }

      std::vector<double> cameraFocalPoint = options.getAsDoubleVector("camera-focal-point");
      if (cameraFocalPoint.size() == 3)
      {
        cam->SetFocalPoint(cameraFocalPoint.data());
      }

      std::vector<double> cameraViewUp = options.getAsDoubleVector("camera-view-up");
      if (cameraViewUp.size() == 3)
      {
        cam->SetViewUp(cameraViewUp.data());
      }

      double cameraViewAngle = options.getAsDouble("camera-view-angle");
      if (cameraViewAngle != 0)
      {
        cam->SetViewAngle(cameraViewAngle);
      }

      cam->Azimuth(options.getAsDouble("camera-azimuth-angle"));
      cam->Elevation(options.getAsDouble("camera-elevation-angle"));
      cam->OrthogonalizeViewUp();

      if (options.getAsBool("verbose"))
      {
        ::DisplayCameraInformation(cam);
      }
    }

    this->Renderer->InitializeCamera();
    return true;
  }

  vtkSmartPointer<vtkRenderWindow> RenWin;
  vtkSmartPointer<vtkF3DRenderer> Renderer;
};

//----------------------------------------------------------------------------
window_impl_standard::window_impl_standard(const options& options, WindowType type)
  : window_impl(options)
  , Internals(new window_impl_standard::F3DInternals)
{
  if (type == WindowType::HEADLESS)
  {
#if F3D_HAS_EGL_SUPPORT
    this->Internals->RenWin = vtkSmartPointer<vtkEGLRenderWindow>::New();
#endif
  }
  else
  {
    this->Internals->RenWin = vtkSmartPointer<vtkRenderWindow>::New();
    this->Internals->RenWin->SetOffScreenRendering(type == WindowType::OFFSCREEN);
  }
  this->Internals->RenWin->SetMultiSamples(0); // Disable hardware antialiasing
}

//----------------------------------------------------------------------------
bool window_impl_standard::setIcon(const void* icon, size_t iconSize)
{
  // XXX This code requires that the interactor has already been set on the render window
  // This is not great, improve VTK on that regard
#if VTK_VERSION_NUMBER >= VTK_VERSION_CHECK(9, 0, 20200615)
  // set icon
  vtkNew<vtkPNGReader> iconReader;
  iconReader->SetMemoryBuffer(icon);
  iconReader->SetMemoryBufferLength(iconSize);
  iconReader->Update();
  this->Internals->RenWin->SetIcon(iconReader->GetOutput());
  return true;
#else
  (void)icon;
  (void)iconSize;
  return false;
#endif
}

//----------------------------------------------------------------------------
bool window_impl_standard::setWindowName(const std::string& windowName)
{
  this->Internals->RenWin->SetWindowName(windowName.c_str());
  return true;
}

//----------------------------------------------------------------------------
window_impl_standard::~window_impl_standard()
{
  if (this->Internals->Renderer)
  {
    // The axis widget should be disabled before calling the renderer destructor
    // As there is a register loop if not
    this->Internals->Renderer->ShowAxis(false);
  }
}

//----------------------------------------------------------------------------
void window_impl_standard::Initialize(bool withColoring, std::string fileInfo)
{
  // Clear renderer if already present
  if (this->Internals->Renderer)
  {
    // Hide axis to make sure the renderer can be deleted if needed
    this->Internals->Renderer->ShowAxis(false);
    this->Internals->RenWin->RemoveRenderer(this->Internals->Renderer);
  }

  this->Internals->RenWin->SetSize(this->Options.getAsIntVector("resolution").data());
  this->Internals->RenWin->SetFullScreen(this->Options.getAsBool("fullscreen"));

  vtkF3DRendererWithColoring* renWithColor =
    vtkF3DRendererWithColoring::SafeDownCast(this->Internals->Renderer);

  // Create the renderer only when needed
  // Note: a vtkF3DRendererWithColoring could always be used instead of switching
  // but it seems more efficient this way
  if (withColoring && !renWithColor)
  {
    this->Internals->Renderer = vtkSmartPointer<vtkF3DRendererWithColoring>::New();
  }
  else if (!withColoring && (renWithColor || !this->Internals->Renderer))
  {
    this->Internals->Renderer = vtkSmartPointer<vtkF3DRenderer>::New();
  }

  this->Internals->RenWin->AddRenderer(this->Internals->Renderer);
  this->Internals->Renderer->Initialize(fileInfo, this->Options.getAsString("up"));
}

//----------------------------------------------------------------------------
bool window_impl_standard::update()
{
  if (this->Internals->Renderer)
  {
    this->Internals->Renderer->ShowAxis(this->Options.getAsBool("axis"));
    this->Internals->Renderer->ShowEdge(this->Options.getAsBool("edges"));
    this->Internals->Renderer->ShowTimer(this->Options.getAsBool("fps"));
    this->Internals->Renderer->ShowFilename(this->Options.getAsBool("filename"));
    this->Internals->Renderer->ShowMetaData(this->Options.getAsBool("metadata"));
    this->Internals->Renderer->SetUseRaytracing(this->Options.getAsBool("raytracing"));
    this->Internals->Renderer->SetRaytracingSamples(this->Options.getAsInt("samples"));
    this->Internals->Renderer->SetUseRaytracingDenoiser(this->Options.getAsBool("denoise"));
    this->Internals->Renderer->SetUseSSAOPass(this->Options.getAsBool("ssao"));
    this->Internals->Renderer->SetUseFXAAPass(this->Options.getAsBool("fxaa"));
    this->Internals->Renderer->SetUseToneMappingPass(this->Options.getAsBool("tone-mapping"));
    this->Internals->Renderer->SetUseBlurBackground(this->Options.getAsBool("blur-background"));
    this->Internals->Renderer->SetUseTrackball(this->Options.getAsBool("trackball"));
    this->Internals->Renderer->SetHDRIFile(this->Options.getAsString("hdri"));
    this->Internals->Renderer->SetUseDepthPeelingPass(this->Options.getAsBool("depth-peeling"));
    this->Internals->Renderer->SetBackground(
      this->Options.getAsDoubleVector("background-color").data());
    this->Internals->Renderer->SetFontFile(this->Options.getAsString("font-file"));

    vtkF3DRendererWithColoring* renWithColor =
      vtkF3DRendererWithColoring::SafeDownCast(this->Internals->Renderer);

    if (renWithColor)
    {
      renWithColor->SetUsePointSprites(this->Options.getAsBool("point-sprites"), false);
      renWithColor->SetUseVolume(this->Options.getAsBool("volume"), false);
      renWithColor->SetUseInverseOpacityFunction(this->Options.getAsBool("inverse"), false);
      renWithColor->ShowScalarBar(this->Options.getAsBool("bar"), false);
      renWithColor->SetScalarBarRange(this->Options.getAsDoubleVector("range"), false);
      renWithColor->SetColormap(this->Options.getAsDoubleVector("colormap"), false);
      renWithColor->UpdateColoringActors();
    }

    // Show grid last as it needs to know the bounding box to be able to compute its size
    this->Internals->Renderer->ShowGrid(this->Options.getAsBool("grid"));

    // Print coloring info when available
    if (this->Options.getAsBool("verbose"))
    {
      f3d::log::info(this->Internals->Renderer->GetRenderingDescription());
    }

    return this->Internals->UpdateCamera(this->Options);
  }
  return false;
}

//----------------------------------------------------------------------------
vtkRenderWindow* window_impl_standard::GetRenderWindow()
{
  return this->Internals->RenWin;
}

//----------------------------------------------------------------------------
bool window_impl_standard::render()
{
  this->Internals->RenWin->Render();
  return true;
}

//----------------------------------------------------------------------------
bool window_impl_standard::renderToFile(const std::string& file, bool noBackground)
{
  return F3DOffscreenRender::RenderOffScreen(this->Internals->RenWin, file, noBackground);
}

//----------------------------------------------------------------------------
bool window_impl_standard::renderAndCompareWithFile(
  const std::string& file, double threshold, bool noBackground, const std::string& outputFile)
{
  return F3DOffscreenRender::RenderTesting(
    this->Internals->RenWin, file, threshold, noBackground, outputFile);
}

//----------------------------------------------------------------------------
void window_impl_standard::InitializeRendererWithColoring(vtkF3DGenericImporter* importer)
{
  vtkF3DRendererWithColoring* renWithColor =
    vtkF3DRendererWithColoring::SafeDownCast(this->Internals->Renderer);
  if (renWithColor && importer)
  {
    renWithColor->SetScalarBarActor(importer->GetScalarBarActor());
    renWithColor->SetGeometryActor(importer->GetGeometryActor());
    renWithColor->SetPointSpritesActor(importer->GetPointSpritesActor());
    renWithColor->SetVolumeProp(importer->GetVolumeProp());
    renWithColor->SetPolyDataMapper(importer->GetPolyDataMapper());
    renWithColor->SetPointGaussianMapper(importer->GetPointGaussianMapper());
    renWithColor->SetVolumeMapper(importer->GetVolumeMapper());
    renWithColor->SetColoring(importer->GetPointDataForColoring(),
      importer->GetCellDataForColoring(), this->Options.getAsBool("cells"),
      importer->GetArrayIndexForColoring(), this->Options.getAsInt("component"));
  }
}
};

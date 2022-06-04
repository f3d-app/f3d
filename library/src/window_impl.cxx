#include "window_impl.h"

#include "log.h"
#include "options.h"

#include "vtkF3DGenericImporter.h"
#include "vtkF3DNoRenderWindow.h"
#include "vtkF3DRendererWithColoring.h"

#include <vtkCamera.h>
#include <vtkExternalOpenGLRenderWindow.h>
#include <vtkImageData.h>
#include <vtkImageExport.h>
#include <vtkPNGReader.h>
#include <vtkPointGaussianMapper.h>
#include <vtkRenderWindow.h>
#include <vtkRendererCollection.h>
#include <vtkVersion.h>
#include <vtkWindowToImageFilter.h>

namespace f3d::detail
{
class window_impl::internals
{
public:
  internals(const options& options)
    : Options(options)
  {
  }

  static void DisplayCameraInformation(vtkCamera* cam)
  {
    double* position = cam->GetPosition();
    log::info("Camera position is: ", position[0], ", ", position[1], ", ", position[2], ".");
    double* focalPoint = cam->GetFocalPoint();
    log::info(
      "Camera focal point is: ", focalPoint[0], ", ", focalPoint[1], ", ", focalPoint[2], ".");
    double* viewUp = cam->GetViewUp();
    log::info("Camera view up is: ", viewUp[0], ", ", viewUp[1], ", ", viewUp[2], ".");
    log::info("Camera view angle is: ", cam->GetViewAngle(), ".\n");
  }

  bool UpdateCamera(const options& options)
  {
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
        window_impl::internals::DisplayCameraInformation(cam);
      }
    }

    this->Renderer->InitializeCamera();
    return true;
  }

  vtkSmartPointer<vtkRenderWindow> RenWin;
  vtkSmartPointer<vtkF3DRenderer> Renderer;
  WindowType Type;
  const options& Options;
};

//----------------------------------------------------------------------------
window_impl::window_impl(const options& options, WindowType type)
  : Internals(new window_impl::internals(options))
{
  this->Internals->Type = type;
  if (type == WindowType::NO_RENDER)
  {
    this->Internals->RenWin = vtkSmartPointer<vtkF3DNoRenderWindow>::New();
  }
  else if (type == WindowType::EXTERNAL)
  {
    vtkNew<vtkExternalOpenGLRenderWindow> renWin;
    renWin->AutomaticWindowPositionAndResizeOff();
    this->Internals->RenWin = renWin;
    this->Internals->RenWin->SetMultiSamples(0); // Disable hardware antialiasing
  }
  else
  {
    this->Internals->RenWin = vtkSmartPointer<vtkRenderWindow>::New();
    this->Internals->RenWin->SetOffScreenRendering(type == WindowType::NATIVE_OFFSCREEN);
    this->Internals->RenWin->SetMultiSamples(0); // Disable hardware antialiasing
  }
}

//----------------------------------------------------------------------------
window_impl::WindowType window_impl::getType()
{
  return this->Internals->Type;
}

//----------------------------------------------------------------------------
bool window_impl::setIcon(const void* icon, size_t iconSize)
{
  // XXX This code requires that the interactor has already been set on the render window
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
bool window_impl::setWindowName(const std::string& windowName)
{
  this->Internals->RenWin->SetWindowName(windowName.c_str());
  return true;
}

//----------------------------------------------------------------------------
window_impl::~window_impl()
{
  if (this->Internals->Renderer)
  {
    // The axis widget should be disabled before calling the renderer destructor
    // As there is a register loop if not
    this->Internals->Renderer->ShowAxis(false);
  }
}

//----------------------------------------------------------------------------
void window_impl::Initialize(bool withColoring, std::string fileInfo)
{
  // Clear renderer if already present
  if (this->Internals->Renderer)
  {
    // Hide axis to make sure the renderer can be deleted if needed
    this->Internals->Renderer->ShowAxis(false);
    this->Internals->RenWin->RemoveRenderer(this->Internals->Renderer);
  }

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
  this->Internals->Renderer->Initialize(fileInfo, this->Internals->Options.getAsString("up"));
}

//----------------------------------------------------------------------------
bool window_impl::update()
{
  if (!this->Internals->Renderer)
  {
    // Renderer is missing, create a default one
    this->Initialize(false, "");
  }

  this->Internals->RenWin->SetSize(this->Internals->Options.getAsIntVector("resolution").data());
  this->Internals->RenWin->SetFullScreen(this->Internals->Options.getAsBool("fullscreen"));

  this->Internals->Renderer->ShowAxis(this->Internals->Options.getAsBool("axis"));
  this->Internals->Renderer->ShowEdge(this->Internals->Options.getAsBool("edges"));
  this->Internals->Renderer->ShowTimer(this->Internals->Options.getAsBool("fps"));
  this->Internals->Renderer->ShowFilename(this->Internals->Options.getAsBool("filename"));
  this->Internals->Renderer->ShowMetaData(this->Internals->Options.getAsBool("metadata"));
  this->Internals->Renderer->SetUseRaytracing(this->Internals->Options.getAsBool("raytracing"));
  this->Internals->Renderer->SetRaytracingSamples(this->Internals->Options.getAsInt("samples"));
  this->Internals->Renderer->SetUseRaytracingDenoiser(
    this->Internals->Options.getAsBool("denoise"));
  this->Internals->Renderer->SetUseSSAOPass(this->Internals->Options.getAsBool("ssao"));
  this->Internals->Renderer->SetUseFXAAPass(this->Internals->Options.getAsBool("fxaa"));
  this->Internals->Renderer->SetUseToneMappingPass(
    this->Internals->Options.getAsBool("tone-mapping"));
  this->Internals->Renderer->SetUseBlurBackground(
    this->Internals->Options.getAsBool("blur-background"));
  this->Internals->Renderer->SetUseTrackball(this->Internals->Options.getAsBool("trackball"));
  this->Internals->Renderer->SetUseDepthPeelingPass(
    this->Internals->Options.getAsBool("depth-peeling"));
  this->Internals->Renderer->SetBackground(
    this->Internals->Options.getAsDoubleVector("background-color").data());
  this->Internals->Renderer->SetHDRIFile(this->Internals->Options.getAsString("hdri"));
  this->Internals->Renderer->SetFontFile(this->Internals->Options.getAsString("font-file"));

  vtkF3DRendererWithColoring* renWithColor =
    vtkF3DRendererWithColoring::SafeDownCast(this->Internals->Renderer);

  if (renWithColor)
  {
    renWithColor->SetUsePointSprites(this->Internals->Options.getAsBool("point-sprites"), false);
    renWithColor->SetUseVolume(this->Internals->Options.getAsBool("volume"), false);
    renWithColor->SetUseInverseOpacityFunction(
      this->Internals->Options.getAsBool("inverse"), false);
    renWithColor->ShowScalarBar(this->Internals->Options.getAsBool("bar"), false);
    renWithColor->SetScalarBarRange(this->Internals->Options.getAsDoubleVector("range"), false);
    renWithColor->SetColormap(this->Internals->Options.getAsDoubleVector("colormap"), false);
    renWithColor->UpdateColoringActors();
  }

  // Show grid last as it needs to know the bounding box to be able to compute its size
  this->Internals->Renderer->ShowGrid(this->Internals->Options.getAsBool("grid"));

  // Print coloring info when available
  if (this->Internals->Options.getAsBool("verbose"))
  {
    log::info(this->Internals->Renderer->GetRenderingDescription());
  }

  return this->Internals->UpdateCamera(this->Internals->Options);
}

//----------------------------------------------------------------------------
vtkRenderWindow* window_impl::GetRenderWindow()
{
  return this->Internals->RenWin;
}

//----------------------------------------------------------------------------
bool window_impl::render()
{
  this->Internals->RenWin->Render();
  return true;
}

//----------------------------------------------------------------------------
image window_impl::renderToImage(bool noBackground)
{
  vtkNew<vtkWindowToImageFilter> rtW2if;
  rtW2if->SetInput(this->Internals->RenWin);

  if (noBackground)
  {
    // we need to set the background to black to avoid blending issues with translucent
    // objects when saving to file with no background
    this->Internals->RenWin->GetRenderers()->GetFirstRenderer()->SetBackground(0, 0, 0);
    rtW2if->SetInputBufferTypeToRGBA();
  }

  vtkNew<vtkImageExport> exporter;
  exporter->SetInputConnection(rtW2if->GetOutputPort());
  exporter->ImageLowerLeftOn();

  int* dims = exporter->GetDataDimensions();
  int cmp = exporter->GetDataNumberOfScalarComponents();

  image output;
  output.setResolution(dims[0], dims[1]);
  output.setChannelCount(cmp);

  exporter->Export(output.getData());

  return output;
}

//----------------------------------------------------------------------------
void window_impl::InitializeRendererWithColoring(vtkF3DGenericImporter* importer)
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
      importer->GetCellDataForColoring(), this->Internals->Options.getAsBool("cells"),
      importer->GetArrayIndexForColoring(), this->Internals->Options.getAsInt("component"));
  }
}
};

#include "window_impl.h"

#include "camera_impl.h"
#include "log.h"
#include "options.h"

#include "vtkF3DConfigure.h"

#include "vtkF3DGenericImporter.h"
#include "vtkF3DNoRenderWindow.h"
#include "vtkF3DRendererWithColoring.h"

#include <vtkCamera.h>
#include <vtkImageData.h>
#include <vtkImageExport.h>
#include <vtkPNGReader.h>
#include <vtkPointGaussianMapper.h>
#include <vtkRenderWindow.h>
#include <vtkRendererCollection.h>
#include <vtkVersion.h>
#include <vtkWindowToImageFilter.h>

#if F3D_MODULE_EXTERNAL_RENDERING
#include <vtkExternalOpenGLRenderWindow.h>
#endif

namespace f3d::detail
{
class window_impl::internals
{
public:
  internals(const options& options)
    : Options(options)
  {
  }

  std::unique_ptr<camera_impl> Camera;
  vtkSmartPointer<vtkRenderWindow> RenWin;
  vtkSmartPointer<vtkF3DRenderer> Renderer;
  Type WindowType;
  const options& Options;
};

//----------------------------------------------------------------------------
window_impl::window_impl(const options& options, Type type)
  : Internals(new window_impl::internals(options))
{
  this->Internals->WindowType = type;
  if (type == Type::NONE)
  {
    this->Internals->RenWin = vtkSmartPointer<vtkF3DNoRenderWindow>::New();
  }
  else if (type == Type::EXTERNAL)
  {
#if F3D_MODULE_EXTERNAL_RENDERING
    this->Internals->RenWin = vtkSmartPointer<vtkExternalOpenGLRenderWindow>::New();
#else
    throw exception("Window type is external but F3D_MODULE_EXTERNAL_RENDERING is not enabled");
#endif
  }
  else
  {
    this->Internals->RenWin = vtkSmartPointer<vtkRenderWindow>::New();
    this->Internals->RenWin->SetOffScreenRendering(type == Type::NATIVE_OFFSCREEN);
    this->Internals->RenWin->SetMultiSamples(0); // Disable hardware antialiasing
  }

  this->Internals->Camera = std::make_unique<detail::camera_impl>();
}

//----------------------------------------------------------------------------
window_impl::Type window_impl::getType()
{
  return this->Internals->WindowType;
}

//----------------------------------------------------------------------------
camera& window_impl::getCamera()
{
  return *this->Internals->Camera;
}

//----------------------------------------------------------------------------
void window_impl::setSize(int width, int height)
{
  this->Internals->RenWin->SetSize(width, height);
}

//----------------------------------------------------------------------------
bool window_impl::setIcon(const void* icon, size_t iconSize)
{
  // SetIcon needs https://gitlab.kitware.com/vtk/vtk/-/merge_requests/7004
#if VTK_VERSION_NUMBER >= VTK_VERSION_CHECK(9, 0, 20200616)
  // XXX This code requires that the interactor has already been set on the render window
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

  this->Internals->Camera->SetVTKRenderer(this->Internals->Renderer);
  this->Internals->RenWin->AddRenderer(this->Internals->Renderer);
  this->Internals->Renderer->Initialize(fileInfo, this->Internals->Options.getAsString("up"));
}

//----------------------------------------------------------------------------
void window_impl::UpdateDynamicOptions()
{
  if (!this->Internals->Renderer)
  {
    // Renderer is missing, create a default one
    this->Initialize(false, "");
  }

  this->Internals->RenWin->SetFullScreen(this->Internals->Options.getAsBool("fullscreen"));

  this->Internals->Renderer->ShowAxis(this->Internals->Options.getAsBool("axis"));
  this->Internals->Renderer->ShowEdge(this->Internals->Options.getAsBool("edges"));
  this->Internals->Renderer->ShowTimer(this->Internals->Options.getAsBool("fps"));
  this->Internals->Renderer->ShowFilename(this->Internals->Options.getAsBool("filename"));
  this->Internals->Renderer->ShowMetaData(this->Internals->Options.getAsBool("metadata"));
  this->Internals->Renderer->ShowCheatSheet(this->Internals->Options.getAsBool("cheatsheet"));
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
    renWithColor->SetColoring(this->Internals->Options.getAsBool("cells"),
      this->Internals->Options.getAsString("scalars"),
      this->Internals->Options.getAsInt("component"));
    renWithColor->SetUsePointSprites(this->Internals->Options.getAsBool("point-sprites"));
    renWithColor->SetUseVolume(this->Internals->Options.getAsBool("volume"));
    renWithColor->SetUseInverseOpacityFunction(this->Internals->Options.getAsBool("inverse"));
    renWithColor->ShowScalarBar(this->Internals->Options.getAsBool("bar"));
    renWithColor->SetScalarBarRange(this->Internals->Options.getAsDoubleVector("range"));
    renWithColor->SetColormap(this->Internals->Options.getAsDoubleVector("colormap"));
    renWithColor->UpdateColoringActors();
  }

  // Show grid last as it needs to know the bounding box to be able to compute its size
  this->Internals->Renderer->ShowGrid(this->Internals->Options.getAsBool("grid"));
}

//----------------------------------------------------------------------------
void window_impl::PrintSceneDescription(log::VerboseLevel level)
{
  log::print(level, this->Internals->Renderer->GetSceneDescription());
}

//----------------------------------------------------------------------------
void window_impl::PrintColoringDescription(log::VerboseLevel level)
{
  vtkF3DRendererWithColoring* renWithColor =
    vtkF3DRendererWithColoring::SafeDownCast(this->Internals->Renderer);
  if (renWithColor)
  {
    log::print(level, renWithColor->GetColoringDescription());
  }
}

//----------------------------------------------------------------------------
vtkRenderWindow* window_impl::GetRenderWindow()
{
  return this->Internals->RenWin;
}

//----------------------------------------------------------------------------
bool window_impl::render()
{
  this->UpdateDynamicOptions();
  this->Internals->RenWin->Render();
  return true;
}

//----------------------------------------------------------------------------
image window_impl::renderToImage(bool noBackground)
{
  this->UpdateDynamicOptions();

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
    renWithColor->SetColoringAttributes(
      importer->GetPointDataForColoring(), importer->GetCellDataForColoring());
  }
}
};

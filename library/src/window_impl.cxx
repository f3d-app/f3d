#include "window_impl.h"

#include "camera_impl.h"
#include "engine.h"
#include "log.h"
#include "macros.h"
#include "options.h"

#include "vtkF3DExternalRenderWindow.h"

#include "vtkF3DGenericImporter.h"
#include "vtkF3DNoRenderWindow.h"
#include "vtkF3DRenderer.h"

#include <vtkCamera.h>
#include <vtkF3DRenderPass.h>
#include <vtkImageData.h>
#include <vtkImageExport.h>
#include <vtkInformation.h>
#include <vtkPNGReader.h>
#include <vtkPointGaussianMapper.h>
#include <vtkRendererCollection.h>
#include <vtkRenderingOpenGLConfigure.h>
#include <vtkVersion.h>
#include <vtkWindowToImageFilter.h>

#ifdef VTK_USE_X
#include <vtkF3DGLXRenderWindow.h>
#endif

#ifdef _WIN32
#include <vtkF3DWGLRenderWindow.h>
#endif

#ifdef VTK_OPENGL_HAS_EGL
#include <vtkF3DEGLRenderWindow.h>
#endif

#if VTK_VERSION_NUMBER >= VTK_VERSION_CHECK(9, 3, 20240914)
#include <vtkOSOpenGLRenderWindow.h>
#endif

#include <sstream>

namespace fs = std::filesystem;

namespace f3d::detail
{
class window_impl::internals
{
public:
  explicit internals(const options& options)
    : Options(options)
  {
  }

  static context::fptr SymbolLoader(void* userptr, const char* name)
  {
    assert(userptr != nullptr);
    auto* fn = static_cast<context::function*>(userptr);
    assert(fn != nullptr);
    return (*fn)(name);
  }

  static vtkSmartPointer<vtkRenderWindow> AutoBackendWindow()
  {
    // Override VTK logic
#ifdef _WIN32
    return vtkSmartPointer<vtkF3DWGLRenderWindow>::New();
#elif defined(__linux__) || defined(__FreeBSD__)
#if defined(VTK_USE_X)
    // try GLX
    vtkSmartPointer<vtkRenderWindow> glxRenWin = vtkSmartPointer<vtkF3DGLXRenderWindow>::New();
    if (glxRenWin)
    {
      return glxRenWin;
    }
#endif
#if defined(VTK_OPENGL_HAS_EGL)
    // try EGL
    vtkSmartPointer<vtkRenderWindow> eglRenWin = vtkSmartPointer<vtkF3DEGLRenderWindow>::New();
    if (eglRenWin)
    {
      return eglRenWin;
    }
#endif
#if VTK_VERSION_NUMBER >= VTK_VERSION_CHECK(9, 3, 20240914)
    // OSMesa
    return vtkSmartPointer<vtkOSOpenGLRenderWindow>::New();
#endif
    return nullptr;
#else
    // fallback on VTK logic for other systems
    return vtkSmartPointer<vtkRenderWindow>::New();
#endif
  }

  std::unique_ptr<camera_impl> Camera;
  vtkSmartPointer<vtkRenderWindow> RenWin;
  vtkNew<vtkF3DRenderer> Renderer;
  const options& Options;
  interactor_impl* Interactor = nullptr;
  fs::path CachePath;
  context::function GetProcAddress;
};

//----------------------------------------------------------------------------
window_impl::window_impl(const options& options, const std::optional<Type>& type, bool offscreen,
  const context::function& getProcAddress)
  : Internals(std::make_unique<window_impl::internals>(options))
{
  this->Internals->GetProcAddress = getProcAddress;
  if (type == Type::NONE)
  {
    this->Internals->RenWin = vtkSmartPointer<vtkF3DNoRenderWindow>::New();
  }
  else if (type == Type::EXTERNAL)
  {
    this->Internals->RenWin = vtkSmartPointer<vtkF3DExternalRenderWindow>::New();
  }
  else if (type == Type::EGL)
  {
#if defined(VTK_OPENGL_HAS_EGL)
    this->Internals->RenWin = vtkSmartPointer<vtkF3DEGLRenderWindow>::New();
#else
    assert(false); // Unreachable
#endif
  }
  else if (type == Type::OSMESA)
  {
#if VTK_VERSION_NUMBER >= VTK_VERSION_CHECK(9, 3, 20240914)
    this->Internals->RenWin = vtkSmartPointer<vtkOSOpenGLRenderWindow>::New();
#else
    throw engine::no_window_exception(
      "Window type is OSMesa but the underlying VTK version is not recent enough to support it");
#endif
  }
  else if (type == Type::GLX)
  {
#if defined(VTK_USE_X)
    this->Internals->RenWin = vtkSmartPointer<vtkF3DGLXRenderWindow>::New();
#else
    assert(false); // Unreachable
#endif
  }
  else if (type == Type::WGL)
  {
#ifdef _WIN32
    this->Internals->RenWin = vtkSmartPointer<vtkF3DWGLRenderWindow>::New();
#else
    assert(false); // Unreachable
#endif
  }
  else if (!type.has_value())
  {
    this->Internals->RenWin = internals::AutoBackendWindow();
  }

  // COCOA and WASM are not handled explicitly
  // as there is no helper method to create them in engine
  if (this->Internals->RenWin == nullptr)
  {
    throw engine::no_window_exception("Cannot create a window");
  }

#if VTK_VERSION_NUMBER >= VTK_VERSION_CHECK(9, 3, 20240914)
  vtkOpenGLRenderWindow* oglRenWin = vtkOpenGLRenderWindow::SafeDownCast(this->Internals->RenWin);
  if (oglRenWin)
  {
    if (this->Internals->GetProcAddress)
    {
      oglRenWin->SetOpenGLSymbolLoader(&internals::SymbolLoader, &this->Internals->GetProcAddress);
    }
  }
#endif

#if VTK_VERSION_NUMBER >= VTK_VERSION_CHECK(9, 3, 20240606)
  this->Internals->RenWin->EnableTranslucentSurfaceOn();
#endif
  this->Internals->RenWin->SetMultiSamples(0); // Disable hardware antialiasing
  this->Internals->RenWin->SetOffScreenRendering(offscreen);
  this->Internals->RenWin->SetWindowName("f3d");
  this->Internals->RenWin->AddRenderer(this->Internals->Renderer);
  this->Internals->Camera = std::make_unique<detail::camera_impl>();
  this->Internals->Camera->SetVTKRenderer(this->Internals->Renderer);

  this->Internals->Renderer->SetConsoleBadgeEnabled(
    !offscreen || std::getenv("CTEST_F3D_CONSOLE_BADGE"));

  this->Initialize();

  log::debug("VTK window class type is ", this->Internals->RenWin->GetClassName());
}

//----------------------------------------------------------------------------
void window_impl::Initialize()
{
  this->Internals->Renderer->Initialize();
}

//----------------------------------------------------------------------------
void window_impl::InitializeUpVector()
{
  this->Internals->Renderer->InitializeUpVector(this->Internals->Options.scene.up_direction);
}

//----------------------------------------------------------------------------
window_impl::Type window_impl::getType()
{
  if (this->Internals->RenWin->IsA("vtkOSOpenGLRenderWindow"))
  {
    return Type::OSMESA;
  }

#ifdef VTK_USE_X
  if (this->Internals->RenWin->IsA("vtkF3DGLXRenderWindow"))
  {
    return Type::GLX;
  }
#endif

#ifdef _WIN32
  if (this->Internals->RenWin->IsA("vtkF3DWGLRenderWindow"))
  {
    return Type::WGL;
  }
#endif

#ifdef __APPLE__
  if (this->Internals->RenWin->IsA("vtkCocoaRenderWindow"))
  {
    return Type::COCOA;
  }
#endif

#ifdef VTK_OPENGL_HAS_EGL
  if (this->Internals->RenWin->IsA("vtkF3DEGLRenderWindow"))
  {
    return Type::EGL;
  }
#endif
#ifdef __EMSCRIPTEN__
  if (this->Internals->RenWin->IsA("vtkWebAssemblyOpenGLRenderWindow"))
  {
    return Type::WASM;
  }
#endif

  if (this->Internals->RenWin->IsA("vtkF3DNoRenderWindow"))
  {
    return Type::NONE;
  }

  return Type::UNKNOWN;
}

//----------------------------------------------------------------------------
bool window_impl::isOffscreen()
{
  return !this->Internals->RenWin->GetShowWindow();
}

//----------------------------------------------------------------------------
camera& window_impl::getCamera()
{
  return *this->Internals->Camera;
}

//----------------------------------------------------------------------------
int window_impl::getWidth() const
{
  return this->Internals->RenWin->GetSize()[0];
}

//----------------------------------------------------------------------------
int window_impl::getHeight() const
{
  return this->Internals->RenWin->GetSize()[1];
}

//----------------------------------------------------------------------------
window& window_impl::setSize(int width, int height)
{
  this->Internals->RenWin->SetSize(width, height);
  return *this;
}

//----------------------------------------------------------------------------
window& window_impl::setPosition(int x, int y)
{
  if (this->Internals->RenWin->IsA("vtkCocoaRenderWindow"))
  {
    // vtkCocoaRenderWindow has a different behavior than other render windows
    // https://gitlab.kitware.com/vtk/vtk/-/issues/18681
    int* screenSize = this->Internals->RenWin->GetScreenSize();
    int* winSize = this->Internals->RenWin->GetSize();
    this->Internals->RenWin->SetPosition(x, screenSize[1] - winSize[1] - y);
  }
  else
  {
    this->Internals->RenWin->SetPosition(x, y);
  }
  return *this;
}

//----------------------------------------------------------------------------
window& window_impl::setIcon(const unsigned char* icon, size_t iconSize)
{
  // XXX This code requires that the interactor has already been set on the render window
  vtkNew<vtkPNGReader> iconReader;
  iconReader->SetMemoryBuffer(icon);
  iconReader->SetMemoryBufferLength(iconSize);
  iconReader->Update();
  this->Internals->RenWin->SetIcon(iconReader->GetOutput());
  return *this;
}

//----------------------------------------------------------------------------
window& window_impl::setWindowName(std::string_view windowName)
{
  this->Internals->RenWin->SetWindowName(windowName.data());
  return *this;
}

//----------------------------------------------------------------------------
point3_t window_impl::getWorldFromDisplay(const point3_t& displayPoint) const
{
  point3_t out = { 0.0, 0.0, 0.0 };
  double worldPt[4];
  this->Internals->Renderer->SetDisplayPoint(displayPoint.data());
  this->Internals->Renderer->DisplayToWorld();
  this->Internals->Renderer->GetWorldPoint(worldPt);

  constexpr double homogeneousThreshold = 1e-7;
  if (worldPt[3] > homogeneousThreshold)
  {
    out[0] = worldPt[0] / worldPt[3];
    out[1] = worldPt[1] / worldPt[3];
    out[2] = worldPt[2] / worldPt[3];
  }
  return out;
}

//----------------------------------------------------------------------------
point3_t window_impl::getDisplayFromWorld(const point3_t& worldPoint) const
{
  point3_t out;
  this->Internals->Renderer->SetWorldPoint(worldPoint[0], worldPoint[1], worldPoint[2], 1.0);
  this->Internals->Renderer->WorldToDisplay();
  this->Internals->Renderer->GetDisplayPoint(out.data());
  return out;
}

//----------------------------------------------------------------------------
window_impl::~window_impl()
{
  // The axis widget should be disabled before calling the renderer destructor
  // As there is a register loop if not
  this->Internals->Renderer->ShowAxis(false);
}

//----------------------------------------------------------------------------
void window_impl::UpdateDynamicOptions()
{
  vtkF3DRenderer* renderer = this->Internals->Renderer;

  if (this->Internals->RenWin->IsA("vtkF3DNoRenderWindow"))
  {
    // With a NONE window type, only update the actors to get accurate bounding box information
    renderer->UpdateActors();
    return;
  }

  // Set the cache path if not already
  renderer->SetCachePath(this->Internals->CachePath.string());

  // Make sure lights are created before we take options into account
  renderer->UpdateLights();

  const options& opt = this->Internals->Options;
  renderer->ShowAxis(opt.ui.axis);
  renderer->SetUseTrackball(opt.interactor.trackball);
  renderer->SetInvertZoom(opt.interactor.invert_zoom);

  // XXX: model.point_sprites.type only has an effect on geometry scene
  // but we set it here for practical reasons
  const int pointSpritesSize = opt.model.point_sprites.size;
  const vtkF3DRenderer::SplatType splatType = opt.model.point_sprites.type == "gaussian"
    ? vtkF3DRenderer::SplatType::GAUSSIAN
    : vtkF3DRenderer::SplatType::SPHERE;
  renderer->SetPointSpritesProperties(splatType, pointSpritesSize);

  renderer->SetLineWidth(opt.render.line_width);
  renderer->SetPointSize(opt.render.point_size);
  renderer->ShowEdge(opt.render.show_edges);
  renderer->ShowTimer(opt.ui.fps);
  renderer->ShowFilename(opt.ui.filename);
  renderer->SetFilenameInfo(opt.ui.filename_info);
  renderer->ShowMetaData(opt.ui.metadata);
  renderer->ShowCheatSheet(opt.ui.cheatsheet);
  renderer->ShowConsole(opt.ui.console);
  renderer->ShowDropZone(opt.ui.dropzone);
  renderer->SetDropZoneInfo(opt.ui.dropzone_info);
  renderer->ShowArmature(opt.render.armature.enable);

  renderer->SetUseRaytracing(opt.render.raytracing.enable);
  renderer->SetRaytracingSamples(opt.render.raytracing.samples);
  renderer->SetUseRaytracingDenoiser(opt.render.raytracing.denoise);

  vtkF3DRenderer::AntiAliasingMode aaMode = vtkF3DRenderer::AntiAliasingMode::NONE;

  // F3D_DEPRECATED
  // Remove this in the next major release
  F3D_SILENT_WARNING_PUSH()
  F3D_SILENT_WARNING_DECL(4996, "deprecated-declarations")
  if (opt.render.effect.anti_aliasing)
  {
    log::warn("render.effect.anti_aliasing is deprecated, please use render.effect.antialiasing.enable instead");
    aaMode = vtkF3DRenderer::AntiAliasingMode::FXAA;
  }
  F3D_SILENT_WARNING_POP()

  if (opt.render.effect.antialiasing.enable)
  {
    if (opt.render.effect.antialiasing.mode == "fxaa")
    {
      aaMode = vtkF3DRenderer::AntiAliasingMode::FXAA;
    }
    else if (opt.render.effect.antialiasing.mode == "ssaa")
    {
      aaMode = vtkF3DRenderer::AntiAliasingMode::SSAA;
    }
    else
    {
      log::warn(opt.render.effect.antialiasing.mode,
        R"( is an invalid antialiasing mode. Valid modes are: "fxaa", "ssaa")");
    }
  }

  renderer->SetUseSSAOPass(opt.render.effect.ambient_occlusion);
  renderer->SetAntiAliasingMode(aaMode);
  renderer->SetUseToneMappingPass(opt.render.effect.tone_mapping);
  renderer->SetUseDepthPeelingPass(opt.render.effect.translucency_support);
  renderer->SetBackfaceType(opt.render.backface_type);
  renderer->SetFinalShader(opt.render.effect.final_shader);

  renderer->SetBackground(opt.render.background.color.data());
  renderer->SetUseBlurBackground(opt.render.background.blur.enable);
  renderer->SetBlurCircleOfConfusionRadius(opt.render.background.blur.coc);
  renderer->SetLightIntensity(opt.render.light.intensity);

  renderer->SetHDRIFile(opt.render.hdri.file);
  renderer->SetUseImageBasedLighting(opt.render.hdri.ambient);
  renderer->ShowHDRISkybox(opt.render.background.skybox);

  renderer->SetFontFile(opt.ui.font_file);
  renderer->SetFontScale(opt.ui.scale);

  renderer->SetGridUnitSquare(opt.render.grid.unit);
  renderer->SetGridSubdivisions(opt.render.grid.subdivisions);
  renderer->SetGridAbsolute(opt.render.grid.absolute);
  renderer->ShowGrid(opt.render.grid.enable);
  renderer->SetGridColor(opt.render.grid.color);

  if (!opt.scene.camera.index.has_value())
  {
    renderer->SetUseOrthographicProjection(opt.scene.camera.orthographic);
  }

  renderer->SetSurfaceColor(opt.model.color.rgb);
  renderer->SetOpacity(opt.model.color.opacity);
  renderer->SetTextureBaseColor(opt.model.color.texture);
  renderer->SetRoughness(opt.model.material.roughness);
  renderer->SetMetallic(opt.model.material.metallic);
  renderer->SetTextureMaterial(opt.model.material.texture);
  renderer->SetTextureEmissive(opt.model.emissive.texture);
  renderer->SetEmissiveFactor(opt.model.emissive.factor);
  renderer->SetTextureNormal(opt.model.normal.texture);
  renderer->SetNormalScale(opt.model.normal.scale);
  renderer->SetTextureMatCap(opt.model.matcap.texture);

  renderer->SetEnableColoring(opt.model.scivis.enable);
  renderer->SetUseCellColoring(opt.model.scivis.cells);
  renderer->SetArrayNameForColoring(opt.model.scivis.array_name);
  renderer->SetComponentForColoring(opt.model.scivis.component);

  renderer->SetScalarBarRange(opt.model.scivis.range);
  renderer->SetColormap(opt.model.scivis.colormap);
  renderer->ShowScalarBar(opt.ui.scalar_bar);

  renderer->SetUsePointSprites(opt.model.point_sprites.enable);
  renderer->SetUseVolume(opt.model.volume.enable);
  renderer->SetUseInverseOpacityFunction(opt.model.volume.inverse);

  renderer->UpdateActors();

  // Update the cheatsheet if needed
  if (this->Internals->Interactor && renderer->CheatSheetNeedsUpdate())
  {
    std::vector<vtkF3DUIActor::CheatSheetGroup> cheatsheet;
    for (const std::string& group : this->Internals->Interactor->getBindGroups())
    {
      std::vector<vtkF3DUIActor::CheatSheetTuple> groupList;
      for (const interaction_bind_t& bind : this->Internals->Interactor->getBindsForGroup(group))
      {
        auto [doc, val] = this->Internals->Interactor->getBindingDocumentation(bind);
        if (!doc.empty())
        {
          groupList.emplace_back(std::make_tuple(bind.format(), doc, val));
        }
      }
      cheatsheet.emplace_back(std::make_pair(group, std::move(groupList)));
    }
    renderer->ConfigureCheatSheet(cheatsheet);
  }
}

//----------------------------------------------------------------------------
void window_impl::PrintSceneDescription(log::VerboseLevel level)
{
  log::print(level, this->Internals->Renderer->GetSceneDescription());
}

//----------------------------------------------------------------------------
void window_impl::PrintColoringDescription(log::VerboseLevel level)
{
  std::string descr = this->Internals->Renderer->GetColoringDescription();
  if (!descr.empty())
  {
    log::print(level, descr);
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
  this->render();

  vtkNew<vtkWindowToImageFilter> rtW2if;
  rtW2if->SetInput(this->Internals->RenWin);

  if (noBackground)
  {
    // we need to set the background to black to avoid blending issues with translucent
    // objects when saving to file with no background
    this->Internals->Renderer->SetBackground(0, 0, 0);
    rtW2if->SetInputBufferTypeToRGBA();
  }

  vtkNew<vtkImageExport> exporter;
  exporter->SetInputConnection(rtW2if->GetOutputPort());
  exporter->ImageLowerLeftOn();

  int* dims = exporter->GetDataDimensions();
  int cmp = exporter->GetDataNumberOfScalarComponents();

  image output(dims[0], dims[1], cmp);
  exporter->Export(output.getContent());

  return output;
}

//----------------------------------------------------------------------------
void window_impl::SetImporter(vtkF3DMetaImporter* importer)
{
  this->Internals->Renderer->SetImporter(importer);
}

//----------------------------------------------------------------------------
void window_impl::SetCachePath(const fs::path& cachePath)
{
  try
  {
    if (cachePath.empty())
    {
      throw engine::cache_exception("Provided cache path is empty");
    }

    // create directories if they do not exist
    fs::create_directories(cachePath);
  }
  catch (const fs::filesystem_error& ex)
  {
    throw engine::cache_exception(std::string("Could not use cache: ") + ex.what());
  }

  this->Internals->CachePath = cachePath;
}

//----------------------------------------------------------------------------
void window_impl::SetInteractor(interactor_impl* interactor)
{
  this->Internals->Interactor = interactor;
}

//----------------------------------------------------------------------------
void window_impl::RenderUIOnly()
{
#if F3D_MODULE_UI
  // Do only a partial render of the UI
  vtkRenderWindow* renWin = this->Internals->RenWin;
  vtkRenderer* ren = renWin->GetRenderers()->GetFirstRenderer();
  vtkInformation* info = ren->GetInformation();

  info->Set(vtkF3DRenderPass::RENDER_UI_ONLY(), 1);
  renWin->Render();
  info->Remove(vtkF3DRenderPass::RENDER_UI_ONLY());
#endif
}
};

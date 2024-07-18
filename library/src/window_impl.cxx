#include "window_impl.h"

#include "camera_impl.h"
#include "engine.h"
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
#include <vtksys/SystemTools.hxx>

#if F3D_MODULE_EXTERNAL_RENDERING
#include <vtkExternalOpenGLRenderWindow.h>
#endif

#ifdef _WIN32
#include <Windows.h>
#include <dwmapi.h>

#ifndef DWMWA_USE_IMMERSIVE_DARK_MODE
#define DWMWA_USE_IMMERSIVE_DARK_MODE 20
#endif

constexpr auto IMMERSIVE_DARK_MODE_SUPPORTED_SINCE = 19041;
#endif

namespace f3d::detail
{
class window_impl::internals
{
public:
  explicit internals(const options& options)
    : Options(options)
  {
  }

  std::string GetCachePath()
  {
    // create directories if they do not exist
    vtksys::SystemTools::MakeDirectory(this->CachePath);

    return this->CachePath;
  }

#if _WIN32
  /**
   * Helper function to detect if the
   * Windows Build Number is equal or greater to a number
   */
  static bool IsWindowsBuildNumberOrGreater(int buildNumber)
  {
    std::string value{};
    bool result = vtksys::SystemTools::ReadRegistryValue(
      "HKEY_LOCAL_MACHINE\\Software\\Microsoft\\Windows NT\\CurrentVersion;CurrentBuildNumber",
      value);

    if (result == true)
    {
      try
      {
        return std::stoi(value) >= buildNumber;
      }
      catch (const std::invalid_argument& e)
      {
        f3d::log::debug("Error parsing CurrentBuildNumber", e.what());
      }
    }
    else
    {
      f3d::log::debug("Error opening registry key.");
    }

    return false;
  }

  /**
   * Helper function to fetch a DWORD from windows registry.
   *
   * @param hKey A handle to an open registry key
   * @param subKey The path of registry key relative to 'hKey'
   * @param value The name of the registry value
   * @param dWord Variable to store the result in
   */
  static bool ReadRegistryDWord(
    HKEY hKey, const std::wstring& subKey, const std::wstring& value, DWORD& dWord)
  {
    DWORD dataSize = sizeof(DWORD);
    LONG result = RegGetValueW(
      hKey, subKey.c_str(), value.c_str(), RRF_RT_REG_DWORD, nullptr, &dWord, &dataSize);

    return result == ERROR_SUCCESS;
  }

  /**
   * Helper function to detect user theme
   */
  static bool IsWindowsInDarkMode()
  {
    std::wstring subKey(L"Software\\Microsoft\\Windows\\CurrentVersion\\Themes\\Personalize");

    DWORD value{};

    if (ReadRegistryDWord(HKEY_CURRENT_USER, subKey, L"AppsUseLightTheme", value))
    {
      return value == 0;
    }

    if (ReadRegistryDWord(HKEY_CURRENT_USER, subKey, L"SystemUsesLightTheme", value))
    {
      return value == 0;
    }

    return false;
  }
#endif

  void UpdateTheme() const
  {
#ifdef _WIN32
    if (this->IsWindowsBuildNumberOrGreater(IMMERSIVE_DARK_MODE_SUPPORTED_SINCE))
    {
      HWND hwnd = static_cast<HWND>(this->RenWin->GetGenericWindowId());
      BOOL useDarkMode = this->IsWindowsInDarkMode();
      DwmSetWindowAttribute(hwnd, DWMWA_USE_IMMERSIVE_DARK_MODE, &useDarkMode, sizeof(useDarkMode));
    }
#endif
  }

  std::unique_ptr<camera_impl> Camera;
  vtkSmartPointer<vtkRenderWindow> RenWin;
  vtkNew<vtkF3DRendererWithColoring> Renderer;
  Type WindowType;
  const options& Options;
  bool Initialized = false;
  bool WithColoring = false;
  std::string CachePath;
};

//----------------------------------------------------------------------------
window_impl::window_impl(const options& options, Type type)
  : Internals(std::make_unique<window_impl::internals>(options))
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
    throw engine::no_window_exception(
      "Window type is external but F3D_MODULE_EXTERNAL_RENDERING is not enabled");
#endif
  }
  else
  {
    this->Internals->RenWin = vtkSmartPointer<vtkRenderWindow>::New();
    this->Internals->RenWin->SetOffScreenRendering(type == Type::NATIVE_OFFSCREEN);
    this->Internals->RenWin->SetMultiSamples(0); // Disable hardware antialiasing

#ifdef __ANDROID__
    // Since F3D_MODULE_EXTERNAL_RENDERING is not supported on Android yet, we need to call
    // this workaround. It makes vtkEGLRenderWindow external if WindowInfo is not nullptr.
    this->Internals->RenWin->SetWindowInfo("jni");
#endif
  }
#if VTK_VERSION_NUMBER >= VTK_VERSION_CHECK(9, 3, 20240606)
  this->Internals->RenWin->EnableTranslucentSurfaceOn();
#endif
  this->Internals->RenWin->SetWindowName("f3d");
  this->Internals->RenWin->AddRenderer(this->Internals->Renderer);
  this->Internals->Camera = std::make_unique<detail::camera_impl>();
  this->Internals->Camera->SetVTKRenderer(this->Internals->Renderer);
}

//----------------------------------------------------------------------------
window_impl::Type window_impl::getType()
{
  return this->Internals->WindowType;
}

//----------------------------------------------------------------------------
camera& window_impl::getCamera()
{
  // Make sure the camera (and the whole rendering stack)
  // is initialized before providing one.
  if (!this->Internals->Initialized)
  {
    this->Initialize(false);
  }

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
window& window_impl::setAnimationNameInfo(const std::string& name)
{
  this->Internals->Renderer->SetAnimationnameInfo(name);
  return *this;
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
window& window_impl::setWindowName(const std::string& windowName)
{
  this->Internals->RenWin->SetWindowName(windowName.c_str());
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
void window_impl::Initialize(bool withColoring)
{
  this->Internals->WithColoring = withColoring;
  this->Internals->Renderer->Initialize(this->Internals->Options.getConstStruct().scene.up_direction);
  this->Internals->UpdateTheme();
  this->Internals->Initialized = true;
}

//----------------------------------------------------------------------------
void window_impl::UpdateDynamicOptions()
{
  if (!this->Internals->Initialized)
  {
    // Renderer is missing, create a default one
    this->Initialize(false);
  }

  if (this->Internals->WindowType == Type::NONE)
  {
    // With a NONE window type, only update the actors to get accurate bounding box information
    this->Internals->Renderer->UpdateActors();
    return;
  }

  // Set the cache path if not already
  this->Internals->Renderer->SetCachePath(this->Internals->GetCachePath());

  // Make sure lights are created before we take options into account
  this->Internals->Renderer->UpdateLights();

  auto optionsStruct = this->Internals->Options.getConstStruct();
  this->Internals->Renderer->ShowAxis(optionsStruct.interactor.axis);
  this->Internals->Renderer->SetUseTrackball(optionsStruct.interactor.trackball);
  this->Internals->Renderer->SetInvertZoom(optionsStruct.interactor.invert_zoom);

  // XXX: model.point-sprites.type only has an effect on geometry scene
  // but we set it here for practical reasons
  std::string splatTypeStr = optionsStruct.model.point_sprites.sprites_type;
  int pointSize = optionsStruct.render.point_size;
  vtkF3DRendererWithColoring::SplatType splatType = vtkF3DRendererWithColoring::SplatType::SPHERE;
  if (splatTypeStr == "gaussian")
  {
    splatType = vtkF3DRendererWithColoring::SplatType::GAUSSIAN;
  }

  this->Internals->Renderer->SetPointProperties(splatType, pointSize);

  this->Internals->Renderer->SetLineWidth(optionsStruct.render.line_width);
  this->Internals->Renderer->ShowEdge(optionsStruct.render.show_edges);
  this->Internals->Renderer->ShowTimer(optionsStruct.ui.fps);
  this->Internals->Renderer->ShowFilename(optionsStruct.ui.filename);
  this->Internals->Renderer->SetFilenameInfo(optionsStruct.ui.filename_info);
  this->Internals->Renderer->ShowMetaData(optionsStruct.ui.metadata);
  this->Internals->Renderer->ShowCheatSheet(optionsStruct.ui.cheatsheet);
  this->Internals->Renderer->ShowDropZone(optionsStruct.ui.dropzone);
  this->Internals->Renderer->SetDropZoneInfo(optionsStruct.ui.dropzone_info);

  this->Internals->Renderer->SetUseRaytracing(optionsStruct.render.raytracing.enable);
  this->Internals->Renderer->SetRaytracingSamples(optionsStruct.render.raytracing.samples);
  this->Internals->Renderer->SetUseRaytracingDenoiser(optionsStruct.render.raytracing.denoise);

  this->Internals->Renderer->SetUseSSAOPass(optionsStruct.render.effect.ambient_occlusion);
  this->Internals->Renderer->SetUseFXAAPass(optionsStruct.render.effect.anti_aliasing);
  this->Internals->Renderer->SetUseToneMappingPass(optionsStruct.render.effect.tone_mapping);
  this->Internals->Renderer->SetUseDepthPeelingPass(
    optionsStruct.render.effect.translucency_support);
  this->Internals->Renderer->SetBackfaceType(optionsStruct.render.backface_type);
  this->Internals->Renderer->SetFinalShader(optionsStruct.render.effect.final_shader);

  this->Internals->Renderer->SetBackground(optionsStruct.render.background.color.data());
  this->Internals->Renderer->SetUseBlurBackground(optionsStruct.render.background.blur);
  this->Internals->Renderer->SetBlurCircleOfConfusionRadius(
    optionsStruct.render.background.blur_coc);
  this->Internals->Renderer->SetLightIntensity(optionsStruct.render.light.intensity);

  std::string hdriFile = optionsStruct.render.hdri.file;
  bool hdriAmbient = optionsStruct.render.hdri.ambient;
  bool hdriSkybox = optionsStruct.render.background.skybox;
  this->Internals->Renderer->SetHDRIFile(hdriFile);
  this->Internals->Renderer->SetUseImageBasedLighting(hdriAmbient);
  this->Internals->Renderer->ShowHDRISkybox(hdriSkybox);

  this->Internals->Renderer->SetFontFile(optionsStruct.ui.font_file);

  this->Internals->Renderer->SetGridUnitSquare(optionsStruct.render.grid.unit);
  this->Internals->Renderer->SetGridSubdivisions(optionsStruct.render.grid.subdivisions);
  this->Internals->Renderer->SetGridAbsolute(optionsStruct.render.grid.absolute);
  this->Internals->Renderer->ShowGrid(optionsStruct.render.grid.enable);
  this->Internals->Renderer->SetGridColor(optionsStruct.render.grid.color);

  if (optionsStruct.scene.camera.index == -1)
  {
    this->Internals->Renderer->SetUseOrthographicProjection(
      optionsStruct.scene.camera.orthographic);
  }

  if (this->Internals->WithColoring)
  {
    this->Internals->Renderer->SetSurfaceColor(optionsStruct.model.color.rgb.data());
    this->Internals->Renderer->SetOpacity(optionsStruct.model.color.opacity);
    this->Internals->Renderer->SetTextureBaseColor(optionsStruct.model.color.texture);
    this->Internals->Renderer->SetRoughness(optionsStruct.model.material.roughness);
    this->Internals->Renderer->SetMetallic(optionsStruct.model.material.metallic);
    this->Internals->Renderer->SetTextureMaterial(optionsStruct.model.material.texture);
    this->Internals->Renderer->SetTextureEmissive(optionsStruct.model.emissive.texture);
    this->Internals->Renderer->SetEmissiveFactor(optionsStruct.model.emissive.factor.data());
    this->Internals->Renderer->SetTextureNormal(optionsStruct.model.normal.texture);
    this->Internals->Renderer->SetNormalScale(optionsStruct.model.normal.scale);
    this->Internals->Renderer->SetTextureMatCap(optionsStruct.model.matcap.texture);

    this->Internals->Renderer->SetColoring(optionsStruct.model.scivis.cells,
      optionsStruct.model.scivis.array_name, optionsStruct.model.scivis.component);
    this->Internals->Renderer->SetScalarBarRange(optionsStruct.model.scivis.range);
    this->Internals->Renderer->SetColormap(optionsStruct.model.scivis.colormap);
    this->Internals->Renderer->ShowScalarBar(optionsStruct.ui.scalar_bar);

    this->Internals->Renderer->SetUsePointSprites(optionsStruct.model.point_sprites.enable);
    this->Internals->Renderer->SetUseVolume(optionsStruct.model.volume.enable);
    this->Internals->Renderer->SetUseInverseOpacityFunction(optionsStruct.model.volume.inverse);
  }

  this->Internals->Renderer->UpdateActors();
}

//----------------------------------------------------------------------------
void window_impl::PrintSceneDescription(log::VerboseLevel level)
{
  log::print(level, this->Internals->Renderer->GetSceneDescription());
}

//----------------------------------------------------------------------------
void window_impl::PrintColoringDescription(log::VerboseLevel level)
{
  if (this->Internals->WithColoring)
  {
    std::string descr = this->Internals->Renderer->GetColoringDescription();
    if (!descr.empty())
    {
      log::print(level, descr);
    }
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

  image output(dims[0], dims[1], cmp);
  exporter->Export(output.getContent());

  return output;
}

//----------------------------------------------------------------------------
void window_impl::SetImporterForColoring(vtkF3DGenericImporter* importer)
{
  if (this->Internals->WithColoring)
  {
    this->Internals->Renderer->SetImporter(importer);
  }
}

//----------------------------------------------------------------------------
void window_impl::SetCachePath(const std::string& cachePath)
{
  this->Internals->CachePath = cachePath;
}
};

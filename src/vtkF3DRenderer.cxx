#include "vtkF3DRenderer.h"

#include "F3DLoader.h"
#include "F3DOptions.h"

#include "vtkF3DOpenGLGridMapper.h"
#include "vtkF3DRenderPass.h"

#include <vtkAbstractArray.h>
#include <vtkActor.h>
#include <vtkActor2DCollection.h>
#include <vtkAxesActor.h>
#include <vtkBoundingBox.h>
#include <vtkCallbackCommand.h>
#include <vtkCamera.h>
#include <vtkCellData.h>
#include <vtkCullerCollection.h>
#include <vtkDataArray.h>
#include <vtkFieldData.h>
#include <vtkImageData.h>
#include <vtkImageReader2.h>
#include <vtkImageReader2Factory.h>
#include <vtkImporter.h>
#include <vtkLightsPass.h>
#include <vtkObjectFactory.h>
#include <vtkOpenGLFXAAPass.h>
#include <vtkOpenGLRenderer.h>
#include <vtkOpenGLTexture.h>
#include <vtkPiecewiseFunction.h>
#include <vtkPointData.h>
#include <vtkProperty.h>
#include <vtkRenderPassCollection.h>
#include <vtkRenderWindowInteractor.h>
#include <vtkRendererCollection.h>
#include <vtkSkybox.h>
#include <vtkTextProperty.h>
#include <vtkToneMappingPass.h>
#include <vtkVersion.h>
#include <vtkVolumeProperty.h>
#include <vtksys/Directory.hxx>
#include <vtksys/SystemTools.hxx>

#include <vtk_glew.h>

#if F3D_HAS_RAYTRACING
#include <vtkOSPRayPass.h>
#include <vtkOSPRayRendererNode.h>
#endif

#include "F3DLog.h"

#include <cctype>
#include <chrono>
#include <cmath>

vtkStandardNewMacro(vtkF3DRenderer);

//----------------------------------------------------------------------------
vtkF3DRenderer::vtkF3DRenderer()
{
  this->Cullers->RemoveAllItems();
}

//----------------------------------------------------------------------------
void vtkF3DRenderer::ReleaseGraphicsResources(vtkWindow* w)
{
  if (this->Timer != 0)
  {
    glDeleteQueries(1, &this->Timer);
    this->Timer = 0;
  }
  this->Superclass::ReleaseGraphicsResources(w);
}

//----------------------------------------------------------------------------
void vtkF3DRenderer::Initialize(const F3DOptions& options, const std::string& fileInfo)
{
  if (!this->RenderWindow)
  {
    F3DLog::Print(F3DLog::Severity::Error, "No render window linked");
    return;
  }

  this->RemoveAllViewProps();
  this->RemoveAllLights();

  this->Options = options;

  this->GridVisible = this->Options.Grid;
  this->AxisVisible = this->Options.Axis;
  this->EdgesVisible = this->Options.Edges;
  this->TimerVisible = this->Options.FPS;
  this->FilenameVisible = this->Options.Filename;
  this->MetaDataVisible = this->Options.MetaData;
  this->ScalarBarVisible = this->Options.Bar;
  this->ScalarsVisible = this->Options.Scalars != f3d::F3DReservedString;
  this->UseRaytracing = this->Options.Raytracing;
  this->UseRaytracingDenoiser = this->Options.Denoise;
  this->UseDepthPeelingPass = this->Options.DepthPeeling;
  this->UseSSAOPass = this->Options.SSAO;
  this->UseFXAAPass = this->Options.FXAA;
  this->UseToneMappingPass = this->Options.ToneMapping;
  this->UsePointSprites = this->Options.PointSprites;
  this->UseVolume = this->Options.Volume;
  this->UseInverseOpacityFunction = this->Options.InverseOpacityFunction;
  this->UseBlurBackground = this->Options.BlurBackground;
  this->UseTrackball = this->Options.Trackball;

  // parse up vector
  if (this->Options.Up.size() == 2)
  {
    char sign = this->Options.Up[0];
    char axis = std::toupper(this->Options.Up[1]);
    if ((sign == '-' || sign == '+') && (axis >= 'X' && axis <= 'Z'))
    {
      this->UpIndex = axis - 'X';
      std::fill(this->UpVector, this->UpVector + 3, 0);
      this->UpVector[this->UpIndex] = (sign == '+') ? 1.0 : -1.0;

      std::fill(this->RightVector, this->RightVector + 3, 0);
      this->RightVector[this->UpIndex == 0 ? 1 : 0] = 1.0;

      double pos[3];
      vtkMath::Cross(this->UpVector, this->RightVector, pos);
      vtkMath::MultiplyScalar(pos, -1.0);

      vtkCamera* cam = this->GetActiveCamera();
      cam->SetFocalPoint(0.0, 0.0, 0.0);
      cam->SetPosition(pos);
      cam->SetViewUp(this->UpVector);

      this->SetEnvironmentUp(this->UpVector);
      this->SetEnvironmentRight(this->RightVector);
    }
  }

  if (!this->Options.HDRIFile.empty() && !this->GetUseImageBasedLighting())
  {
    std::string fullPath = vtksys::SystemTools::CollapseFullPath(this->Options.HDRIFile);

    auto reader = vtkSmartPointer<vtkImageReader2>::Take(
      vtkImageReader2Factory::CreateImageReader2(fullPath.c_str()));
    if (reader)
    {
      reader->SetFileName(fullPath.c_str());
      reader->Update();

      vtkNew<vtkTexture> texture;
      texture->SetColorModeToDirectScalars();
      texture->MipmapOn();
      texture->InterpolateOn();
      texture->SetInputConnection(reader->GetOutputPort());

      // 8-bit textures are usually gamma-corrected
      if (reader->GetOutput() && reader->GetOutput()->GetScalarType() == VTK_UNSIGNED_CHAR)
      {
        texture->UseSRGBColorSpaceOn();
      }

      // HDRI OpenGL
      this->UseImageBasedLightingOn();
      this->SetEnvironmentTexture(texture);

      // Skybox OpenGL
      double front[3];
      vtkMath::Cross(this->RightVector, this->UpVector, front);
      this->Skybox->SetFloorPlane(this->UpVector[0], this->UpVector[1], this->UpVector[2], 0.0);
      this->Skybox->SetFloorRight(front[0], front[1], front[2]);
      this->Skybox->SetProjection(vtkSkybox::Sphere);
      this->Skybox->SetTexture(texture);

// First version of VTK including the version check (and the feature used)
#if VTK_VERSION_NUMBER >= VTK_VERSION_CHECK(9, 0, 20200527)
      this->Skybox->GammaCorrectOn();
#endif
    }
    else
    {
      vtkWarningMacro("Cannot open HDRI file " << fullPath);
    }
  }

  if (this->GetUseImageBasedLighting())
  {
    this->AddActor(this->Skybox);
    this->AutomaticLightCreationOff();
  }
  else
  {
    if (this->Options.NoBackground && !this->Options.Output.empty())
    {
      // we need to set the background to black to avoid blending issues with translucent
      // objetcs when saving to file with no background
      this->SetBackground(0, 0, 0);
    }
    else
    {
      this->SetBackground(this->Options.BackgroundColor[0], this->Options.BackgroundColor[1],
        this->Options.BackgroundColor[2]);
    }
    this->AutomaticLightCreationOn();
  }

  double textColor[3];
  if (this->IsBackgroundDark())
  {
    textColor[0] = textColor[1] = textColor[2] = 1.0;
  }
  else
  {
    textColor[0] = textColor[1] = textColor[2] = 0.0;
  }

  this->FilenameActor->SetText(vtkCornerAnnotation::UpperEdge, fileInfo.c_str());
  this->FilenameActor->GetTextProperty()->SetFontFamilyToCourier();
  this->FilenameActor->GetTextProperty()->SetColor(textColor);

  this->MetaDataActor->GetTextProperty()->SetFontFamilyToCourier();
  this->MetaDataActor->GetTextProperty()->SetFontSize(15);
  this->MetaDataActor->GetTextProperty()->SetOpacity(0.5);
  this->MetaDataActor->GetTextProperty()->SetBackgroundColor(0, 0, 0);
  this->MetaDataActor->GetTextProperty()->SetBackgroundOpacity(0.5);

  this->TimerActor->GetTextProperty()->SetFontFamilyToCourier();
  this->TimerActor->GetTextProperty()->SetColor(textColor);
  this->TimerActor->GetTextProperty()->SetFontSize(15);
  this->TimerActor->SetPosition(10, 10);

  this->CheatSheetActor->GetTextProperty()->SetFontFamilyToCourier();
  this->CheatSheetActor->GetTextProperty()->SetFontSize(15);
  this->CheatSheetActor->GetTextProperty()->SetOpacity(0.5);
  this->CheatSheetActor->GetTextProperty()->SetBackgroundColor(0, 0, 0);
  this->CheatSheetActor->GetTextProperty()->SetBackgroundOpacity(0.5);

  this->TimerActor->SetInput("0 fps");

  this->SetupRenderPasses();
}

//----------------------------------------------------------------------------
std::string vtkF3DRenderer::GenerateMetaDataDescription()
{
  std::string description;
  description += " \n";
  if (this->PolyDataMapper)
  {
    vtkDataSet* dataset = this->PolyDataMapper->GetInput();
    if (dataset)
    {
      description += " Number of points: ";
      description += std::to_string(dataset->GetNumberOfPoints());
      description += " \n Number of cells: ";
      description += std::to_string(dataset->GetNumberOfCells());
      description += " \n";

      // Field Data
      vtkFieldData* fieldData = dataset->GetFieldData();
      int nbArrays = fieldData->GetNumberOfArrays();
      for (vtkIdType i = 0; i < nbArrays; i++)
      {
        vtkAbstractArray* array = fieldData->GetAbstractArray(i);
        if (array)
        {
          vtkIdType nbTuples = array->GetNumberOfTuples();
          if (nbTuples == 1)
          {
            description += " ";
            description += array->GetName();
            description += " = ";
            description += array->GetVariantValue(0).ToString();
            description += " \n";
          }
        }
      }
    }
    else
    {
      description += " Unavailable\n";
    }
  }
  else
  {
    description += " Unavailable\n";
  }

  return description;
}

//----------------------------------------------------------------------------
void vtkF3DRenderer::SetupRenderPasses()
{
  // clean up previous pass
  vtkRenderPass* pass = this->GetPass();
  if (pass)
  {
    pass->ReleaseGraphicsResources(this->RenderWindow);
  }

  vtkNew<vtkF3DRenderPass> newPass;
  newPass->SetUseRaytracing(F3D_HAS_RAYTRACING && this->UseRaytracing);
  newPass->SetUseSSAOPass(this->UseSSAOPass);
  newPass->SetUseDepthPeelingPass(this->UseDepthPeelingPass);
  newPass->SetUseBlurBackground(this->UseBlurBackground);

  double bounds[6];
  this->ComputeVisiblePropBounds(bounds);
  newPass->SetBounds(bounds);

  // Image post processing passes
  vtkSmartPointer<vtkRenderPass> renderingPass = newPass;

  if (this->UseToneMappingPass)
  {
    vtkNew<vtkToneMappingPass> toneP;
    toneP->SetToneMappingType(vtkToneMappingPass::GenericFilmic);
    toneP->SetGenericFilmicDefaultPresets();
    toneP->SetDelegatePass(renderingPass);
    renderingPass = toneP;
  }

  if (this->UseFXAAPass)
  {
    vtkNew<vtkOpenGLFXAAPass> fxaaP;
    fxaaP->SetDelegatePass(renderingPass);

    this->SetPass(fxaaP);
    renderingPass = fxaaP;
  }

  this->SetPass(renderingPass);

#if F3D_HAS_RAYTRACING
  vtkOSPRayRendererNode::SetRendererType("pathtracer", this);
  vtkOSPRayRendererNode::SetSamplesPerPixel(this->Options.Samples, this);
  vtkOSPRayRendererNode::SetEnableDenoiser(this->UseRaytracingDenoiser, this);
  vtkOSPRayRendererNode::SetDenoiserThreshold(0, this);

  bool hasHDRI = this->GetEnvironmentTexture() != nullptr;
  vtkOSPRayRendererNode::SetBackgroundMode(hasHDRI ? 2 : 1, this);
#endif
}

//----------------------------------------------------------------------------
void vtkF3DRenderer::ShowAxis(bool show)
{
  if (show)
  {
    vtkNew<vtkAxesActor> axes;
    this->AxisWidget = vtkSmartPointer<vtkOrientationMarkerWidget>::New();
    this->AxisWidget->SetOrientationMarker(axes);
    this->AxisWidget->SetInteractor(this->RenderWindow->GetInteractor());
    this->AxisWidget->SetViewport(0.85, 0.0, 1.0, 0.15);
    this->AxisWidget->On();
    this->AxisWidget->InteractiveOff();
    this->AxisWidget->SetKeyPressActivation(false);
  }
  else
  {
    this->AxisWidget = nullptr;
  }

  this->AxisVisible = show;
  this->SetupRenderPasses();
  this->CheatSheetNeedUpdate = true;
}

//----------------------------------------------------------------------------
bool vtkF3DRenderer::IsAxisVisible()
{
  return this->AxisVisible;
}

//----------------------------------------------------------------------------
void vtkF3DRenderer::ShowGrid(bool show)
{
  this->GridVisible = show;

  double bounds[6];
  this->ComputeVisiblePropBounds(bounds);

  vtkBoundingBox bbox(bounds);

  if (bbox.IsValid())
  {
    this->SetClippingRangeExpansion(0.99);

    double diag = bbox.GetDiagonalLength();
    double unitSquare = pow(10.0, round(log10(diag * 0.1)));

    double gridPos[3];
    for (int i=0; i<3; i++)
    {
      double size = bounds[2*i+1] - bounds[2*i];
      gridPos[i] = 0.5 * (bounds[2*i] + bounds[2*i+1] - this->UpVector[i] * size);
    }

    if (this->Options.Verbose && show)
    {
      F3DLog::Print(F3DLog::Severity::Info, "Using grid unit square size = ", unitSquare, "\n",
        "Grid origin set to [", gridPos[0], ", ", gridPos[1], ", ", gridPos[2], "]\n");
    }

    vtkNew<vtkF3DOpenGLGridMapper> gridMapper;
    gridMapper->SetFadeDistance(diag);
    gridMapper->SetUnitSquare(unitSquare);
    gridMapper->SetUpIndex(this->UpIndex);

    this->GridActor->GetProperty()->SetColor(0.0, 0.0, 0.0);
    this->GridActor->ForceTranslucentOn();
    this->GridActor->SetPosition(gridPos);
    this->GridActor->SetMapper(gridMapper);
    this->GridActor->UseBoundsOff();

    this->RemoveActor(this->GridActor);
    this->AddActor(this->GridActor);
  }
  else
  {
    this->SetClippingRangeExpansion(0);
    show = false;
  }
  this->GridActor->SetVisibility(show);
  this->ResetCameraClippingRange();
  this->SetupRenderPasses();
  this->CheatSheetNeedUpdate = true;
}

//----------------------------------------------------------------------------
bool vtkF3DRenderer::IsGridVisible()
{
  return this->GridVisible;
}

//----------------------------------------------------------------------------
void vtkF3DRenderer::SetUseDepthPeelingPass(bool use)
{
  this->UseDepthPeelingPass = use;
  this->SetupRenderPasses();
  this->CheatSheetNeedUpdate = true;
}

//----------------------------------------------------------------------------
bool vtkF3DRenderer::UsingBlurBackground()
{
  return this->UseBlurBackground;
}

//----------------------------------------------------------------------------
void vtkF3DRenderer::SetUseBlurBackground(bool use)
{
  this->UseBlurBackground = use;
  this->SetupRenderPasses();
  this->CheatSheetNeedUpdate = true;
}

//----------------------------------------------------------------------------
bool vtkF3DRenderer::UsingDepthPeelingPass()
{
  return this->UseDepthPeelingPass;
}

//----------------------------------------------------------------------------
void vtkF3DRenderer::SetUseSSAOPass(bool use)
{
  this->UseSSAOPass = use;
  this->SetupRenderPasses();
  this->CheatSheetNeedUpdate = true;
}

//----------------------------------------------------------------------------
bool vtkF3DRenderer::UsingSSAOPass()
{
  return this->UseSSAOPass;
}

//----------------------------------------------------------------------------
void vtkF3DRenderer::SetUseFXAAPass(bool use)
{
  this->UseFXAAPass = use;
  this->SetupRenderPasses();
  this->CheatSheetNeedUpdate = true;
}

//----------------------------------------------------------------------------
bool vtkF3DRenderer::UsingFXAAPass()
{
  return this->UseFXAAPass;
}

//----------------------------------------------------------------------------
void vtkF3DRenderer::SetUseToneMappingPass(bool use)
{
  this->UseToneMappingPass = use;
  this->SetupRenderPasses();
  this->CheatSheetNeedUpdate = true;
}

//----------------------------------------------------------------------------
bool vtkF3DRenderer::UsingToneMappingPass()
{
  return this->UseToneMappingPass;
}

//----------------------------------------------------------------------------
void vtkF3DRenderer::SetUsePointSprites(bool use)
{
  this->UsePointSprites = use;
  this->UpdateActorsVisibility();
  this->SetupRenderPasses();
  this->CheatSheetNeedUpdate = true;
}

//----------------------------------------------------------------------------
bool vtkF3DRenderer::UsingPointSprites()
{
  return this->UsePointSprites;
}

//----------------------------------------------------------------------------
void vtkF3DRenderer::SetUseVolume(bool use)
{
  this->UseVolume = use;
  this->UpdateActorsVisibility();
  this->SetupRenderPasses();
  this->CheatSheetNeedUpdate = true;
}

//----------------------------------------------------------------------------
bool vtkF3DRenderer::UsingVolume()
{
  return this->UseVolume;
}

//----------------------------------------------------------------------------
void vtkF3DRenderer::SetUseInverseOpacityFunction(bool use)
{
  this->UseInverseOpacityFunction = use;
  if (this->VolumeProp)
  {
    vtkPiecewiseFunction* pwf = this->VolumeProp->GetProperty()->GetScalarOpacity();
    if (pwf->GetSize() == 2)
    {
      double range[2];
      pwf->GetRange(range);

      pwf->RemoveAllPoints();
      pwf->AddPoint(range[0], this->UseInverseOpacityFunction ? 1.0 : 0.0);
      pwf->AddPoint(range[1], this->UseInverseOpacityFunction ? 0.0 : 1.0);
    }
    this->SetupRenderPasses();
  }
  this->CheatSheetNeedUpdate = true;
}

//----------------------------------------------------------------------------
bool vtkF3DRenderer::UsingInverseOpacityFunction()
{
  return this->UseInverseOpacityFunction;
}

//----------------------------------------------------------------------------
void vtkF3DRenderer::SetUseRaytracing(bool use)
{
  this->UseRaytracing = use;
  this->UpdateActorsVisibility();
  this->SetupRenderPasses();
  this->CheatSheetNeedUpdate = true;
}

//----------------------------------------------------------------------------
bool vtkF3DRenderer::UsingRaytracing()
{
  return this->UseRaytracing;
}

//----------------------------------------------------------------------------
void vtkF3DRenderer::SetUseRaytracingDenoiser(bool use)
{
  this->UseRaytracingDenoiser = use;
  this->SetupRenderPasses();
  this->CheatSheetNeedUpdate = true;
}

//----------------------------------------------------------------------------
bool vtkF3DRenderer::UsingRaytracingDenoiser()
{
  return this->UseRaytracingDenoiser;
}

//----------------------------------------------------------------------------
void vtkF3DRenderer::ShowScalars(bool show)
{
  this->ScalarsVisible = show;
  if (this->GeometryActor && this->PointGaussianMapper && this->PolyDataMapper)
  {
    if (show && !this->ScalarsAvailable)
    {
      F3DLog::Print(F3DLog::Severity::Error,
        "Cannot show scalars with this dataset or with the requested array");
      show = false;
    }
    this->PolyDataMapper->SetScalarVisibility(show);
    this->PointGaussianMapper->SetScalarVisibility(show);
    this->UpdateScalarBarVisibility();
  }
  this->SetupRenderPasses();
  this->CheatSheetNeedUpdate = true;
}

//----------------------------------------------------------------------------
bool vtkF3DRenderer::AreScalarsVisible()
{
  return this->ScalarsVisible;
}

//----------------------------------------------------------------------------
void vtkF3DRenderer::ShowScalarBar(bool show)
{
  this->ScalarBarVisible = show;
  this->UpdateScalarBarVisibility();
  this->SetupRenderPasses();
  this->CheatSheetNeedUpdate = true;
}

//----------------------------------------------------------------------------
bool vtkF3DRenderer::IsScalarBarVisible()
{
  return this->ScalarBarVisible;
}

//----------------------------------------------------------------------------
void vtkF3DRenderer::ShowTimer(bool show)
{
  if (this->TimerActor)
  {
    this->RemoveActor(this->TimerActor);
    this->AddActor(this->TimerActor);
    this->TimerActor->SetVisibility(show);
  }
  this->TimerVisible = show;
  this->SetupRenderPasses();
  this->CheatSheetNeedUpdate = true;
}

//----------------------------------------------------------------------------
bool vtkF3DRenderer::IsTimerVisible()
{
  return this->TimerVisible;
}

//----------------------------------------------------------------------------
void vtkF3DRenderer::ShowFilename(bool show)
{
  if (this->FilenameActor)
  {
    this->RemoveActor(this->FilenameActor);
    this->AddActor(this->FilenameActor);
    this->FilenameActor->SetVisibility(show);
  }
  this->FilenameVisible = show;
  this->SetupRenderPasses();
  this->CheatSheetNeedUpdate = true;
}

//----------------------------------------------------------------------------
bool vtkF3DRenderer::IsFilenameVisible()
{
  return this->FilenameVisible;
}

//----------------------------------------------------------------------------
void vtkF3DRenderer::ShowMetaData(bool show)
{
  if (this->MetaDataActor)
  {
    this->RemoveActor(this->MetaDataActor);
    this->AddActor(this->MetaDataActor);

    // generate field data description
    std::string MetaDataDesc = this->GenerateMetaDataDescription();
    this->MetaDataActor->SetText(vtkCornerAnnotation::RightEdge, MetaDataDesc.c_str());

    this->MetaDataActor->SetVisibility(show);
  }
  this->MetaDataVisible = show;
  this->SetupRenderPasses();
  this->CheatSheetNeedUpdate = true;
}

//----------------------------------------------------------------------------
bool vtkF3DRenderer::IsMetaDataVisible()
{
  return this->MetaDataVisible;
}

//----------------------------------------------------------------------------
void vtkF3DRenderer::ShowCheatSheet(bool show)
{
  if (this->CheatSheetActor)
  {
    this->RemoveActor(this->CheatSheetActor);

    if (show)
    {
      this->AddActor(this->CheatSheetActor);
      this->CheatSheetActor->SetVisibility(show);
    }
  }
  this->CheatSheetVisible = show;
  this->SetupRenderPasses();
  this->CheatSheetNeedUpdate = true;
}

//----------------------------------------------------------------------------
bool vtkF3DRenderer::IsCheatSheetVisible()
{
  return this->CheatSheetVisible;
}

//----------------------------------------------------------------------------
void vtkF3DRenderer::UpdateCheatSheet()
{
  if (this->CheatSheetVisible)
  {
    std::stringstream cheatSheetText;
    cheatSheetText << "\n S: Scalars coloring " << (this->ScalarsVisible ? "[ON]" : "[OFF]")
                   << "\n";
    cheatSheetText << " B: Scalar bar " << (this->ScalarBarVisible ? "[ON]" : "[OFF]") << "\n";
    cheatSheetText << " P: Depth peeling " << (this->UseDepthPeelingPass ? "[ON]" : "[OFF]")
                   << "\n";
    cheatSheetText << " Q: SSAO " << (this->UseSSAOPass ? "[ON]" : "[OFF]") << "\n";
    cheatSheetText << " A: FXAA " << (this->UseFXAAPass ? "[ON]" : "[OFF]") << "\n";
    cheatSheetText << " T: Tone mapping " << (this->UseToneMappingPass ? "[ON]" : "[OFF]") << "\n";
    cheatSheetText << " E: Edge visibility " << (this->EdgesVisible ? "[ON]" : "[OFF]") << "\n";
    cheatSheetText << " X: Axis " << (this->AxisVisible ? "[ON]" : "[OFF]") << "\n";
    cheatSheetText << " G: Grid " << (this->GridVisible ? "[ON]" : "[OFF]") << "\n";
    cheatSheetText << " N: File name " << (this->FilenameVisible ? "[ON]" : "[OFF]") << "\n";
    cheatSheetText << " M: Metadata " << (this->MetaDataVisible ? "[ON]" : "[OFF]") << "\n";
    cheatSheetText << " Z: FPS Timer " << (this->TimerVisible ? "[ON]" : "[OFF]") << "\n";
    cheatSheetText << " R: Raytracing " << (this->UseRaytracing ? "[ON]" : "[OFF]") << "\n";
    cheatSheetText << " D: Denoiser " << (this->UseRaytracingDenoiser ? "[ON]" : "[OFF]") << "\n";
    cheatSheetText << " V: Volume representation " << (this->UseVolume ? "[ON]" : "[OFF]") << "\n";
    cheatSheetText << " I: Inverse volume opacity "
                   << (this->UseInverseOpacityFunction ? "[ON]" : "[OFF]") << "\n";
    cheatSheetText << " O: Point sprites " << (this->UsePointSprites ? "[ON]" : "[OFF]") << "\n";
    cheatSheetText << " F: Full screen "
                   << (this->GetRenderWindow()->GetFullScreen() ? "[ON]" : "[OFF]") << "\n";
    cheatSheetText << " U: Blur background " << (this->UseBlurBackground ? "[ON]" : "[OFF]")
                   << "\n";
    cheatSheetText << " K: Trackball interaction" << (this->UseTrackball ? "[ON]" : "[OFF]") << "\n";
    cheatSheetText << "\n H  : Cheat sheet \n";
    cheatSheetText << " ?  : Dump camera state to the terminal\n";
    cheatSheetText << " ESC : Quit \n";
    cheatSheetText << " ENTER: Reset camera \n";
    cheatSheetText << "\n   H  : Cheat sheet \n";
    cheatSheetText << "   ?  : Dump camera state to the terminal\n";
    cheatSheetText << "  ESC : Quit \n";
    cheatSheetText << " ENTER: Reset camera to initial parameters\n";
    cheatSheetText << " SPACE: Play animation if any\n";
    cheatSheetText << " LEFT : Previous file \n";
    cheatSheetText << " RIGHT: Next file \n";
    cheatSheetText << " UP   : Reload current file \n";

    this->CheatSheetActor->SetText(vtkCornerAnnotation::LeftEdge, cheatSheetText.str().c_str());
    this->CheatSheetActor->RenderOpaqueGeometry(this);
  }
}

//----------------------------------------------------------------------------
void vtkF3DRenderer::ShowEdge(bool show)
{
  vtkActor* anActor;
  vtkActorCollection* ac = this->GetActors();
  vtkCollectionSimpleIterator ait;
  for (ac->InitTraversal(ait); (anActor = ac->GetNextActor(ait));)
  {
    if (vtkSkybox::SafeDownCast(anActor) == nullptr)
    {
      anActor->GetProperty()->SetEdgeVisibility(show);
    }
  }
  this->EdgesVisible = show;
  this->CheatSheetNeedUpdate = true;
}

//----------------------------------------------------------------------------
bool vtkF3DRenderer::IsEdgeVisible()
{
  return this->EdgesVisible;
}

//----------------------------------------------------------------------------
void vtkF3DRenderer::SetUseTrackball(bool use)
{
  this->UseTrackball = use;
  this->CheatSheetNeedUpdate = true;
}

//----------------------------------------------------------------------------
bool vtkF3DRenderer::UsingTrackball()
{
  return this->UseTrackball;
}

//----------------------------------------------------------------------------
void vtkF3DRenderer::ShowOptions()
{
  this->ShowGrid(this->GridVisible);
  this->ShowAxis(this->AxisVisible);
  this->ShowScalars(this->ScalarsVisible);
  this->ShowTimer(this->TimerVisible);
  this->ShowEdge(this->EdgesVisible);
  this->ShowFilename(this->FilenameVisible);
  this->ShowCheatSheet(this->CheatSheetVisible);
  this->ShowMetaData(this->MetaDataVisible);

  // Set the initial camera once all options
  // have been shown as they may have an effect on it
  vtkCamera* cam = this->GetActiveCamera();
  this->InitialCamera->DeepCopy(cam);
}

//----------------------------------------------------------------------------
void vtkF3DRenderer::Render()
{
  if (this->CheatSheetNeedUpdate)
  {
    this->UpdateCheatSheet();
    this->CheatSheetNeedUpdate = false;
  }

  if (!this->TimerVisible)
  {
    this->Superclass::Render();
    return;
  }

  auto cpuStart = std::chrono::high_resolution_clock::now();
  if (this->Timer == 0)
  {
    glGenQueries(1, &this->Timer);
  }

  glBeginQuery(GL_TIME_ELAPSED, this->Timer);

  this->TimerActor->RenderOpaqueGeometry(this); // update texture

  this->Superclass::Render();

  auto cpuElapsed = std::chrono::high_resolution_clock::now() - cpuStart;

  // Get CPU frame per seconds
  int fps = static_cast<int>(std::round(
    1.0 / (std::chrono::duration_cast<std::chrono::microseconds>(cpuElapsed).count() * 1e-6)));

  glEndQuery(GL_TIME_ELAPSED);
  GLint elapsed;
  glGetQueryObjectiv(this->Timer, GL_QUERY_RESULT, &elapsed);

  // Get min between CPU frame per seconds and GPU frame per seconds
  fps = std::min(fps, static_cast<int>(std::round(1.0 / (elapsed * 1e-9))));

  std::string str = std::to_string(fps);
  str += " fps";
  this->TimerActor->SetInput(str.c_str());
}

//----------------------------------------------------------------------------
void vtkF3DRenderer::InitializeCamera()
{
  this->Superclass::ResetCamera();
  vtkCamera* cam = this->GetActiveCamera();
  if (this->Options.CameraPosition.size() == 3)
  {
    cam->SetPosition(this->Options.CameraPosition.data());
  }
  if (this->Options.CameraFocalPoint.size() == 3)
  {
    cam->SetFocalPoint(this->Options.CameraFocalPoint.data());
  }
  if (this->Options.CameraViewUp.size() == 3)
  {
    cam->SetViewUp(this->Options.CameraViewUp.data());
  }
  if (this->Options.CameraViewAngle != 0)
  {
    cam->SetViewAngle(this->Options.CameraViewAngle);
  }
  cam->OrthogonalizeViewUp();
  if (this->Options.Verbose)
  {
    double* position = cam->GetPosition();
    F3DLog::Print(F3DLog::Severity::Info, "Camera position is : ", position[0], ", ", position[1],
      ", ", position[2], ".");
    double* focalPoint = cam->GetFocalPoint();
    F3DLog::Print(F3DLog::Severity::Info, "Camera focal point is : ", focalPoint[0], ", ",
      focalPoint[1], ", ", focalPoint[2], ".");
    double* viewUp = cam->GetViewUp();
    F3DLog::Print(F3DLog::Severity::Info, "Camera view up is : ", viewUp[0], ", ", viewUp[1], ", ",
      viewUp[2], ".");
    F3DLog::Print(F3DLog::Severity::Info, "Camera view angle is : ", cam->GetViewAngle(), ".\n");
  }
}

//----------------------------------------------------------------------------
void vtkF3DRenderer::ResetCamera()
{
  vtkCamera* cam = this->GetActiveCamera();
  cam->DeepCopy(this->InitialCamera);
  cam->Modified();
}

//----------------------------------------------------------------------------
void vtkF3DRenderer::UpdateActorsVisibility()
{
  if (this->GeometryActor)
  {
    this->GeometryActor->SetVisibility(
      this->UseRaytracing || (!this->UseVolume && !this->UsePointSprites));
  }
  if (this->PointSpritesActor)
  {
    this->PointSpritesActor->SetVisibility(
      !this->UseRaytracing && !this->UseVolume && this->UsePointSprites);
  }
  if (this->VolumeProp)
  {
    bool visibility = !this->UseRaytracing && this->UseVolume;
    vtkSmartVolumeMapper* mapper =
      vtkSmartVolumeMapper::SafeDownCast(this->VolumeProp->GetMapper());
    if (visibility && (!mapper || !mapper->GetInput() || !this->ScalarsAvailable))
    {
      F3DLog::Print(
        F3DLog::Severity::Error, "Cannot use volume with this dataset or with the requested array");
      visibility = false;
    }
    this->VolumeProp->SetVisibility(visibility);
  }
  this->UpdateScalarBarVisibility();
}

//----------------------------------------------------------------------------
void vtkF3DRenderer::UpdateScalarBarVisibility()
{
  if (this->ScalarBarActor)
  {
    this->ScalarBarActor->SetVisibility(this->ScalarBarVisible && this->ScalarsAvailable &&
      (this->ScalarsVisible || this->UseVolume));
  }
}

//----------------------------------------------------------------------------
bool vtkF3DRenderer::IsBackgroundDark()
{
  double luminance =
    0.299 * this->Background[0] + 0.587 * this->Background[1] + 0.114 * this->Background[2];
  return luminance < 0.5;
}

//----------------------------------------------------------------------------
void vtkF3DRenderer::DumpSceneState()
{
  vtkCamera* cam = this->GetActiveCamera();
  double position[3];
  double focal[3];
  double up[3];
  cam->GetPosition(position);
  cam->GetFocalPoint(focal);
  cam->GetViewUp(up);
  F3DLog::Print(
    F3DLog::Severity::Info, "Camera position: ", position[0], ",", position[1], ",", position[2]);
  F3DLog::Print(
    F3DLog::Severity::Info, "Camera focal point: ", focal[0], ",", focal[1], ",", focal[2]);
  F3DLog::Print(F3DLog::Severity::Info, "Camera view up: ", up[0], ",", up[1], ",", up[2], "\n");
}

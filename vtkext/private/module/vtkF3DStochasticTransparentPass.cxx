#include "vtkF3DStochasticTransparentPass.h"

#include "vtkF3DRandomFS.h"

#include <vtkF3DOpenGLGridMapper.h>
#include <vtkInformation.h>
#include <vtkInformationIntegerKey.h>
#include <vtkObjectFactory.h>
#include <vtkOpenGLActor.h>
#include <vtkOpenGLPointGaussianMapperHelper.h>
#include <vtkOpenGLRenderWindow.h>
#include <vtkRenderState.h>
#include <vtkRenderer.h>
#include <vtkShaderProgram.h>
#include <vtkVersion.h>

#if VTK_VERSION_NUMBER >= VTK_VERSION_CHECK(9, 5, 20251120)
#include "vtkF3DPointSplatMapper.h"
#endif

vtkStandardNewMacro(vtkF3DStochasticTransparentPass);
vtkCxxSetObjectMacro(vtkF3DStochasticTransparentPass, TranslucentPass, vtkRenderPass);
vtkCxxSetObjectMacro(vtkF3DStochasticTransparentPass, VolumetricPass, vtkRenderPass);

vtkInformationKeyMacro(vtkF3DStochasticTransparentPass, PropIndex, Integer);

//------------------------------------------------------------------------------
vtkF3DStochasticTransparentPass::vtkF3DStochasticTransparentPass() = default;

//------------------------------------------------------------------------------
vtkF3DStochasticTransparentPass::~vtkF3DStochasticTransparentPass()
{
  if (this->TranslucentPass)
  {
    this->TranslucentPass->Delete();
  }
  if (this->VolumetricPass)
  {
    this->VolumetricPass->Delete();
  }
}

//------------------------------------------------------------------------------
void vtkF3DStochasticTransparentPass::Render(const vtkRenderState* s)
{
  this->RenWin = static_cast<vtkOpenGLRenderWindow*>(s->GetRenderer()->GetRenderWindow());

  // Setup vtkOpenGLRenderPass
  this->PreRender(s);

  // force usage of depth buffer, even if the actors are translucents
  int numProps = s->GetPropArrayCount();
  for (int j = 0; j < numProps; ++j)
  {
    vtkProp* prop = s->GetPropArray()[j];
    vtkInformation* info = prop->GetPropertyKeys();
    info->Set(vtkOpenGLActor::GLDepthMaskOverride(), 1);
    info->Set(vtkF3DStochasticTransparentPass::PropIndex(), j);
  }

  this->TranslucentPass->Render(s);
  this->VolumetricPass->Render(s);

  this->PostRender(s);

  // remove depth buffer keys
  for (int j = 0; j < numProps; ++j)
  {
    vtkProp* prop = s->GetPropArray()[j];
    vtkInformation* info = prop->GetPropertyKeys();
    info->Remove(vtkOpenGLActor::GLDepthMaskOverride());
  }
}

//------------------------------------------------------------------------------
bool vtkF3DStochasticTransparentPass::SetShaderParameters(vtkShaderProgram* program,
  vtkAbstractMapper* mapper, vtkProp* prop, vtkOpenGLVertexArrayObject* VAO)
{
  vtkInformation* info = prop->GetPropertyKeys();
  program->SetUniformi("propIndex", info->Get(vtkF3DStochasticTransparentPass::PropIndex()));
  program->SetUniformi("seed", this->Seed++);

  return this->Superclass::SetShaderParameters(program, mapper, prop, VAO);
}

//------------------------------------------------------------------------------
bool vtkF3DStochasticTransparentPass::PreReplaceShaderValues(std::string& vtkNotUsed(vertexShader),
  std::string& vtkNotUsed(geometryShader), std::string& fragmentShader, vtkAbstractMapper* mapper,
  vtkProp* vtkNotUsed(prop))
{
  if (!vtkF3DOpenGLGridMapper::SafeDownCast(mapper))
  {
    // add random function utilities
    std::string dec = vtkF3DRandomFS;
    dec += "\nuniform int propIndex;\n";
    dec += "\nuniform int seed;\n";

    std::string primIdDef = "\n#define primId gl_PrimitiveID\n";

    // If we are using instancing, use the instance id instead of the primitive id
#if VTK_VERSION_NUMBER >= VTK_VERSION_CHECK(9, 5, 20251120)
    vtkOpenGLPointGaussianMapperHelper* splatHelper =
      vtkOpenGLPointGaussianMapperHelper::SafeDownCast(mapper);

    if (splatHelper)
    {
      vtkF3DPointSplatMapper* splatMapper =
        vtkF3DPointSplatMapper::SafeDownCast(splatHelper->Owner);

      if (splatMapper && splatMapper->GetUseInstancing())
      {
        primIdDef = "\n#define primId instanceId\n";
      }
    }
#endif

    dec += primIdDef;

    vtkShaderProgram::Substitute(fragmentShader, "//VTK::Color::Dec", dec);

    vtkShaderProgram::Substitute(fragmentShader, "  //VTK::Color::Impl",
      "  //VTK::Color::Impl\n"
      "  float rd = random_ign(gl_FragCoord.xy, hash(uvec3(seed, propIndex, primId)));\n"
      "  if (rd >= opacity) discard;\n"
      "  opacity = 1.0;\n\n");
  }

  return true;
}

//------------------------------------------------------------------------------
void vtkF3DStochasticTransparentPass::ReleaseGraphicsResources(vtkWindow* win)
{
  this->Superclass::ReleaseGraphicsResources(win);

  if (this->TranslucentPass)
  {
    this->TranslucentPass->ReleaseGraphicsResources(win);
  }
  if (this->VolumetricPass)
  {
    this->VolumetricPass->ReleaseGraphicsResources(win);
  }
}

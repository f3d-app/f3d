#include "vtkF3DHookShaderRenderPass.h"

#include <vtkObjectFactory.h>
#include <vtkRenderState.h>

#include <cassert>

vtkStandardNewMacro(vtkF3DHookShaderRenderPass);

// ----------------------------------------------------------------------------
void vtkF3DHookShaderRenderPass::SetDelegatePass(vtkRenderPass* delegate)
{
  if (this->DelegatePass != delegate)
  {
    this->DelegatePass = delegate;
    this->Modified();
  }
}

// ----------------------------------------------------------------------------
void vtkF3DHookShaderRenderPass::Render(const vtkRenderState* s)
{
  assert(this->DelegatePass);

  this->NumberOfRenderedProps = 0;

  this->PreRender(s);
  this->DelegatePass->Render(s);
  this->NumberOfRenderedProps = this->DelegatePass->GetNumberOfRenderedProps();
  this->PostRender(s);
}

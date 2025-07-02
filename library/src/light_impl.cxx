#include "light_impl.h"
#include <memory>
#include <vtkLight.h>
#include <vtkMath.h>

namespace f3d::detail
{
class light_impl::internals
{
public:
  vtkLight* VtkLight;
  light_state_t DefaultLight;
};

//----------------------------------------------------------------------------
light_impl::light_impl()
  : Internals(std::make_unique<light_impl::internals>())
{
}

//----------------------------------------------------------------------------
light_impl::~light_impl() = default;

//----------------------------------------------------------------------------
light& light_impl::setType(const light_type& type)
{
  this->Internals->VtkLight->SetLightType(type);
  return *this;
}

//----------------------------------------------------------------------------
light_type light_impl::getType() const
{
  return static_cast<light_type>(this->Internals->VtkLight->GetLightType());
}

//----------------------------------------------------------------------------
light& light_impl::setPosition(const point3_t& pos)
{
  this->Internals->VtkLight->SetPosition(pos[0], pos[1], pos[2]);
  return *this;
}

//----------------------------------------------------------------------------
light& light_impl::setPositionalLight(bool positional)
{
  this->Internals->VtkLight->SetPositional(positional);
  return *this;
}

//----------------------------------------------------------------------------
bool light_impl::isPositionalLight() const
{
  return this->Internals->VtkLight->GetPositional();
}

//----------------------------------------------------------------------------
point3_t light_impl::getPosition() const
{
  point3_t pos;
  double* vtkPos = this->Internals->VtkLight->GetPosition();
  pos[0] = vtkPos[0];
  pos[1] = vtkPos[1];
  pos[2] = vtkPos[2];
  return pos;
}

//----------------------------------------------------------------------------
light& light_impl::setColor(const color_t& col)
{
  this->Internals->VtkLight->SetColor(col[0], col[1], col[2]);
  return *this;
}

//----------------------------------------------------------------------------
color_t light_impl::getColor() const
{
  color_t col;
  double* vtkCol = this->Internals->VtkLight->GetDiffuseColor();
  col[0] = vtkCol[0];
  col[1] = vtkCol[1];
  col[2] = vtkCol[2];
  return col;
}

//----------------------------------------------------------------------------
light& light_impl::setDirection(const vector3_t& dir)
{
  auto position = this->Internals->VtkLight->GetPosition();
  this->Internals->VtkLight->SetFocalPoint(
    position[0] + dir[0], position[1] + dir[1], position[2] + dir[2]);
  return *this;
}

//----------------------------------------------------------------------------
vector3_t light_impl::getDirection() const
{
  vector3_t dir;
  auto position = this->Internals->VtkLight->GetPosition();
  auto focalPoint = this->Internals->VtkLight->GetFocalPoint();

  dir[0] = focalPoint[0] - position[0];
  dir[1] = focalPoint[1] - position[1];
  dir[2] = focalPoint[2] - position[2];

  vtkMath::Normalize(dir.data());
  return dir;
}

//----------------------------------------------------------------------------
light& light_impl::setIntensity(double intensity)
{
  this->Internals->VtkLight->SetIntensity(intensity);
  return *this;
}

//----------------------------------------------------------------------------
double light_impl::getIntensity() const
{
  return this->Internals->VtkLight->GetIntensity();
}

//----------------------------------------------------------------------------
light& light_impl::setState(const light_state_t& state)
{
  this->setType(state.type)
    .setPosition(state.position)
    .setColor(state.color)
    .setDirection(state.direction)
    .setPositionalLight(state.positionalLight)
    .setIntensity(state.intensity);
  return *this;
}

//----------------------------------------------------------------------------
light_state_t light_impl::getState() const
{
  light_state_t state;
  state.type = this->getType();
  state.position = this->getPosition();
  state.color = this->getColor();
  state.direction = this->getDirection();
  state.positionalLight = this->isPositionalLight();
  state.intensity = this->getIntensity();
  return state;
}

//----------------------------------------------------------------------------
void light_impl::SetVTKLight(vtkLight* lightObj)
{
  this->Internals->VtkLight = lightObj;
}

//----------------------------------------------------------------------------
vtkLight* light_impl::GetVTKLight()
{
  return this->Internals->VtkLight;
}

//----------------------------------------------------------------------------
light& light_impl::resetToDefault()
{
  this->Internals->DefaultLight = this->getState();
  return *this;
}
}
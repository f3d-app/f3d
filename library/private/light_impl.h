/**
 * @class light_impl
 * @brief Implementation of the light
 *
 * A concrete implementation of light that can provide an array of lights to be controlled in a
 * window.
 */
#ifndef f3d_light_impl_h
#define f3d_light_impl_h

#include "light.h"

#include <memory>

class vtkLight;
namespace f3d::detail
{
class light_impl : public light
{
public:
  /**
   * Default constructor
   */
  light_impl();

  /**
   * Default destructor
   */
  ~light_impl() override;

  ///@{
  /**
   * Documented public API
   */
  light& setType(const light_type& type) override;
  light_type getType() const override;
  light& setPositionalLight(bool positional) override;
  bool isPositionalLight() const override;
  light& setPosition(const point3_t& pos) override;
  point3_t getPosition() const override;
  light& setColor(const color_t& col) override;
  color_t getColor() const override;
  light& setDirection(const vector3_t& dir) override;
  vector3_t getDirection() const override;
  light& setIntensity(double intensity) override;
  double getIntensity() const override;
  light& setState(const light_state_t& state) override;
  [[nodiscard]] light_state_t getState() const override;

  light& resetToDefault() override;
  ///@}

  /**
   * Implementation only API.
   */
  void SetVTKLight(vtkLight* lightObj);

  /**
   * Implementation only API.
   */
  [[nodiscard]]
  vtkLight* GetVTKLight();

private:
  class internals;
  std::unique_ptr<internals> Internals;
};
}

#endif

#ifndef f3d_light_h
#define f3d_light_h
#include "export.h"
#include "types.h"

namespace f3d
{
enum F3D_EXPORT light_type
{
  HEADLIGHT = 1,
  CAMERA_LIGHT = 2,
  SCENE_LIGHT = 3,
};

struct F3D_EXPORT light_state_t
{
  light_type type = SCENE_LIGHT;
  point3_t position = { 0., 0., 0. };
  color_t color = { 1., 1., 1. };
  vector3_t direction = { 0., 0., 0. };
  bool positionalLight = false;
  double intensity = 1.0;
};

/**
 * @class   light
 * @brief   Abstract class to control a light in a scene
 *
 * A class to control lights in a window.
 * Angles are in degrees.
 */
class F3D_EXPORT light
{
public:
  /// @name Parameters
  /**
   * Set/Get the light parameters.
   */
  virtual light& setType(const light_type& lightType) = 0;
  [[nodiscard]] virtual light_type getType() const = 0;
  virtual light& setPosition(const point3_t& pos) = 0;
  [[nodiscard]] virtual point3_t getPosition() const = 0;
  virtual light& setColor(const color_t& col) = 0;
  [[nodiscard]] virtual color_t getColor() const = 0;
  virtual light& setPositionalLight(bool positional) = 0;
  [[nodiscard]] virtual bool isPositionalLight() const = 0;
  virtual light& setDirection(const vector3_t& dir) = 0;
  [[nodiscard]] virtual vector3_t getDirection() const = 0;
  virtual light& setIntensity(double intensity) = 0;
  [[nodiscard]] virtual double getIntensity() const = 0;
  virtual light& setState(const light_state_t& state) = 0;
  [[nodiscard]] virtual light_state_t getState() const = 0;

  /**
   * Reset the light to its default state.
   */
  virtual light& resetToDefault() = 0;

protected:
  //! @cond
  light() = default;
  virtual ~light() = default;
  light(const light&) = delete;
  light(light&&) = delete;
  light& operator=(const light&) = delete;
  light& operator=(light&&) = delete;
  //! @endcond
};
}
#endif
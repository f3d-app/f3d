/**
 * @class   scene_impl
 * @brief   A concrete implementation of scene
 *
 * A concrete implementation of scene that hides the private API
 * See scene.h for the class documentation
 */

#ifndef f3d_scene_impl_h
#define f3d_scene_impl_h

#include "scene.h"

#include <memory>

namespace f3d
{
class options;

namespace detail
{
class interactor_impl;
class window_impl;
class scene_impl : public scene
{
public:
  ///@{
  /**
   * Documented public API
   */
  scene_impl(const options& options, window_impl& window);
  ~scene_impl();
  scene& add(const std::filesystem::path& filePath) override;
  scene& add(const std::vector<std::filesystem::path>& filePath) override;
  scene& add(const std::vector<std::string>& filePathStrings) override;
  scene& add(const mesh_t& mesh) override;
  scene& clear() override;
  bool supports(const std::filesystem::path& filePath) override;
  ///@}

  /**
   * Implementation only API.
   * Set the interactor to use when interacting and set the AnimationManager on the interactor.
   */
  void SetInteractor(interactor_impl* interactor);

private:
  class internals;
  std::unique_ptr<internals> Internals;
};
}
}

#endif

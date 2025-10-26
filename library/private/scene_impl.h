/**
 * @class   scene_impl
 * @brief   A concrete implementation of scene
 *
 * A concrete implementation of scene that hides the private API
 * See scene.h for the class documentation
 */

#ifndef f3d_scene_impl_h
#define f3d_scene_impl_h

#include "log.h"
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
  scene_impl(options& options, window_impl& window);
  ~scene_impl();
  scene& add(const std::filesystem::path& filePath) override;
  scene& add(const std::vector<std::filesystem::path>& filePath) override;
  scene& add(const std::vector<std::string>& filePathStrings) override;
  scene& add(const mesh_t& mesh) override;
  scene& clear() override;
  int addLight(const light_state_t& lightState) const override;
  int getLightCount() const override;
  light_state_t getLight(int index) const override;
  scene& updateLight(int index, const light_state_t& lightState) override;
  scene& removeLight(int index) override;
  scene& removeAllLights() override;
  bool supports(const std::filesystem::path& filePath) override;
  scene& loadAnimationTime(double timeValue) override;
  std::pair<double, double> animationTimeRange() override;
  unsigned int availableAnimations() const override;
  ///@}

  /**
   * Implementation only API.
   * Set the interactor to use when interacting and set the AnimationManager on the interactor.
   */
  void SetInteractor(interactor_impl* interactor);

  /**
   * Display available cameras in the log
   */
  void PrintImporterDescription(log::VerboseLevel level);

private:
  class internals;
  std::unique_ptr<internals> Internals;

public:
  /**
   * Get scene hierarchy nodes for display and control
   */
  std::vector<NodeInfo> GetSceneHierarchyNodes() override;
};
}
}

#endif

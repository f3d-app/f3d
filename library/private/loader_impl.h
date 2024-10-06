/**
 * @class   loader_impl
 * @brief   A concrete implementation of loader
 *
 * A concrete implementation of loader that hides the private API
 * See loader.h for the class documentation
 */

#ifndef f3d_loader_impl_h
#define f3d_loader_impl_h

#include "loader.h"

#include <memory>

namespace f3d
{
class options;

namespace detail
{
class interactor_impl;
class window_impl;
class loader_impl : public loader
{
public:
  ///@{
  /**
   * Documented public API
   */
  loader_impl(const options& options, window_impl& window);
  ~loader_impl();
  loader& add(const std::filesystem::path& filePath) override;
  loader& add(const std::vector<std::filesystem::path>& filePath) override;
  loader& add(const std::vector<std::string>& filePathStrings) override;
  loader& add(const mesh_t& mesh) override;
  loader& clear() override;
  bool supported(const std::filesystem::path& filePath) override;
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

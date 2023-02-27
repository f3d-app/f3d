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
//  bool loadFile(const std::string& filePath) override;
  loader& setFilenameInfo(const std::string& filenameInfo) override;
  bool addGeometry(const std::string& filePath) override;
  loader& resetToDefaultScene() override;
  bool loadFullScene(const std::string& filePath) override;
  bool canReadScene(const std::string& filePath) override;
  bool canReadGeometry(const std::string& filePath) override;

  ///@}

  /**
   * Implementation only API.
   * Set the interactor to use when interacting.
   */
  void setInteractor(interactor_impl* interactor);

private:
  class internals;
  std::unique_ptr<internals> Internals;
};
}
}

#endif

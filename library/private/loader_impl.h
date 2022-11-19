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

  loader& addFiles(const std::vector<std::string>& files, bool recursive = false) override;
  loader& addFile(const std::string& path, bool recursive = false) override;

  const std::vector<std::string>& getFiles() const override;

  loader& setCurrentFileIndex(int index) override;
  int getCurrentFileIndex() const override;

  bool loadFile(LoadFileEnum load) override;

  void getFileInfo(LoadFileEnum load, int& nextFileIndex, std::string& filePath,
    std::string& fileInfo) const override;
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

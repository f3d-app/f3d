#ifndef f3d_window_impl_standard_h
#define f3d_window_impl_standard_h

#include "window_impl.h"

#include <memory>
#include <string>

// TODO Doc
class vtkRenderWindow;
class vtkF3DGenericImporter;
namespace f3d
{
class loader;

namespace detail
{
class window_impl_standard : public window_impl
{
public:
  window_impl_standard(const options& options, bool offscreen);
  ~window_impl_standard() override;

  bool update() override;
  bool render() override;
  image renderToImage(bool noBackground = false) override;
  bool setIcon(const void* icon, size_t iconSize) override;
  bool setWindowName(const std::string& windowName) override;

  void Initialize(bool withColoring, std::string fileInfo) override;
  vtkRenderWindow* GetRenderWindow() override;
  void InitializeRendererWithColoring(vtkF3DGenericImporter* importer) override;

private:
  class internals;
  std::unique_ptr<internals> Internals;
};
}
}

#endif

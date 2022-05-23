#ifndef f3d_window_impl_h
#define f3d_window_impl_h

#include "window.h"

// TODO Doc
class vtkRenderWindow;
class vtkF3DGenericImporter;
namespace f3d
{
class options;

namespace detail
{
class window_impl : public window
{
public:
  ~window_impl() override = default;

  bool update() override;
  bool render() override;
  image renderToImage(bool noBackground = false) override;
  bool setIcon(const void* icon, size_t iconSize) override;
  bool setWindowName(const std::string& windowName) override;

  // Private API
  virtual void Initialize(bool withColoring, std::string fileInfo);
  virtual void InitializeRendererWithColoring(vtkF3DGenericImporter* importer);
  virtual vtkRenderWindow* GetRenderWindow() = 0;

protected:
  window_impl(const options&);

  const options& Options;
};
}
}

#endif

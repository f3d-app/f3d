#ifndef f3d_window_impl_standard_h
#define f3d_window_impl_standard_h

#include "f3d_window_impl.h"

#include <memory>
#include <string>

// TODO Doc
class vtkRenderWindow;
class vtkF3DGenericImporter;
namespace f3d
{
class loader;
class window_impl_standard : public window_impl
{
public:
  window_impl_standard(const options& options, bool offscreen);
  ~window_impl_standard() override;

  bool update() override;
  bool render() override;
  bool renderToFile(const std::string& file, bool noBackground = false) override;
  bool renderAndCompareWithFile(const std::string& file, double threshold,
    bool noBackground = false, const std::string& outputFile = "") override;
  bool setIcon(const void* icon, size_t iconSize) override;
  bool setWindowName(const std::string& windowName) override;

  void Initialize(bool withColoring, std::string fileInfo) override;
  vtkRenderWindow* GetRenderWindow() override;
  void InitializeRendererWithColoring(vtkF3DGenericImporter* importer) override;

private:
  class F3DInternals;
  std::unique_ptr<F3DInternals> Internals;
};
}

#endif

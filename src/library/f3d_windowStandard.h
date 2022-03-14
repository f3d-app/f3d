#ifndef f3d_windowStandard_h
#define f3d_windowStandard_h

#include "f3d_window.h"

#include <memory>
#include <string>

class vtkRenderWindow;
class vtkF3DGenericImporter;
namespace f3d
{
class loader;
class windowStandard : public window
{
public:
  windowStandard(const options& options, bool offscreen);
  ~windowStandard() override;

  bool update() override;
  bool render() override;
  bool renderToFile(const std::string& file, bool noBackground = false) override;
  bool renderAndCompareWithFile(const std::string& file, double threshold,
    bool noBackground = false, const std::string& outputFile = "") override;
  bool setIcon(const void* icon, size_t iconSize) override;
  bool setWindowName(const std::string& windowName) override;

  // TODO Private API
  void Initialize(bool withColoring, std::string fileInfo) override;
  vtkRenderWindow* GetRenderWindow() override;
  void InitializeRendererWithColoring(vtkF3DGenericImporter* importer) override;

private:
  class F3DInternals;
  std::unique_ptr<F3DInternals> Internals;
};
}

#endif

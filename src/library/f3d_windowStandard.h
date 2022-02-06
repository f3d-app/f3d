#ifndef f3d_windowStandard_h
#define f3d_windowStandard_h

#include "f3d_window.h"

#include <memory>
#include <string>

class vtkRenderWindow;
class vtkF3DRenderer;
namespace f3d
{
class loader;
class windowStandard : public window
{
public:
  windowStandard(const std::string& windowName, bool offscreen, const void* icon, size_t iconSize);
  ~windowStandard();

  void update() override;
  void render() override;

  // TODO Private API
  void Initialize(bool withColoring, std::string fileInfo) override;
  vtkRenderWindow* GetRenderWindow() override;
  vtkF3DRenderer* GetRenderer() override;

private:
  class F3DInternals;
  std::unique_ptr<F3DInternals> Internals;
};
}

#endif

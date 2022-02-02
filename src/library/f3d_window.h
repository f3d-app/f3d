#ifndef f3d_window_h
#define f3d_window_h

#include <memory>
#include <string>

class vtkRenderWindow;
class vtkF3DRenderer;
namespace f3d
{
class loader;
class window
{
public:
  window(const std::string& windowName, bool offscreen, const void* icon, size_t iconSize);
  ~window();

  // XXX is this needed ?
  //  window(const window& opt);
  //  window& operator=(const window& opt);

  void update();

  // TODO Private API
  void SetLoader(f3d::loader* loader);
  void Initialize(bool withColoring, std::string fileInfo);
  vtkRenderWindow* GetRenderWindow();
  vtkF3DRenderer* GetRenderer();

private:
  class F3DInternals;
  std::unique_ptr<F3DInternals> Internals;
};
}

#endif

#ifndef f3d_window_h
#define f3d_window_h

#include <string>

class vtkRenderWindow;
class vtkF3DRenderer;
namespace f3d
{
class options;
class window
{
public:
  virtual ~window() = default;
  
  virtual void update();

  // TODO Private API
  virtual void SetOptions(const f3d::options* options);
  virtual void Initialize(bool withColoring, std::string fileInfo);
  virtual vtkRenderWindow* GetRenderWindow() = 0;
  virtual vtkF3DRenderer* GetRenderer();

protected:
  window() = default;

  // XXX is this needed ?
  //  window(const window& opt);
  //  window& operator=(const window& opt);\

  const f3d::options* Options;
};
}

#endif

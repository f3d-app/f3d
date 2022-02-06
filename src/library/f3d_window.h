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

  virtual bool update();
  virtual bool updateCamera();
  virtual bool render();
  virtual bool renderToFile(const std::string& file, bool noBackground = false);
  virtual bool renderAndCompareWithFile(const std::string& file, double threshold, bool noBackground, const std::string& outputFile = "");

  // TODO Private API
  virtual void SetOptions(const f3d::options* options);
  virtual void Initialize(bool withColoring, std::string fileInfo);
  virtual vtkRenderWindow* GetRenderWindow() = 0;
  virtual vtkF3DRenderer* GetRenderer();

protected:
  window() = default;

  const f3d::options* Options;
};
}

#endif

#ifndef f3d_window_h
#define f3d_window_h

#include <string>

class vtkRenderWindow;
class vtkF3DGenericImporter;
namespace f3d
{
class options;
class window
{
public:
  virtual ~window() = default;

  virtual bool update();
  virtual bool render();
  virtual bool renderToFile(const std::string& file, bool noBackground = false);
  virtual bool renderAndCompareWithFile(const std::string& file, double threshold,
    bool noBackground, const std::string& outputFile = "");
  virtual void setIcon(const void* icon, size_t iconSize){};

  // TODO Private API
  void SetOptions(const f3d::options* options);
  virtual void Initialize(bool withColoring, std::string fileInfo);
  virtual void InitializeRendererWithColoring(vtkF3DGenericImporter* importer);
  virtual vtkRenderWindow* GetRenderWindow() = 0;

protected:
  window() = default;

  const f3d::options* Options;
};
}

#endif

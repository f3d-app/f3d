/**
 * @class   F3DOffscreenRender
 * @brief   The offscreen rendering utility class
 *
 */

#ifndef F3DOffscreenRender_h
#define F3DOffscreenRender_h

#include <string>

class vtkRenderWindow;

class F3DOffscreenRender
{
public:
  static int RenderOffScreen(vtkRenderWindow* renWin, const std::string& output);
  static int RenderTesting(vtkRenderWindow* renWin, const std::string& reference, double threshold);
};

#endif

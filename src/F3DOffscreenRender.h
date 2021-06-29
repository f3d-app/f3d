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
  /**
   * Render renWin into output png provided.
   * Returns true if sucessful, false otherwise.
   */
  static bool RenderOffScreen(vtkRenderWindow* renWin, const std::string& output, bool noBg = false);

  /**
   * Render renWin and compare with reference using provided threshold. Output png is generated in case of failure.
   * Returns true if sucessful, false otherwise.
   */
  static bool RenderTesting(vtkRenderWindow* renWin, const std::string& reference, double threshold,
    const std::string& output);
};

#endif

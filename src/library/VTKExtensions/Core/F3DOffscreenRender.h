/**
 * @class   F3DOffscreenRender
 * @brief   The offscreen rendering utility namespace
 *
 */

#ifndef F3DOffscreenRender_h
#define F3DOffscreenRender_h

#include <string>

class vtkImageData;
class vtkRenderWindow;

namespace F3DOffscreenRender
{
/**
 * Render renWin into a vtkImageData.
 * Returns true if successful, false otherwise.
 */
bool RenderToImage(vtkRenderWindow* renWin, vtkImageData* image, bool noBg = false);

/**
 * Render renWin into output png provided.
 * Returns true if successful, false otherwise.
 */
bool RenderOffScreen(vtkRenderWindow* renWin, const std::string& output, bool noBg = false);

/**
 * Render renWin and compare with reference using provided threshold. Output png is generated in
 * case of failure. Returns true if successful, false otherwise.
 */
bool RenderTesting(vtkRenderWindow* renWin, const std::string& reference, double threshold,
  bool noBg, const std::string& output);
};

#endif

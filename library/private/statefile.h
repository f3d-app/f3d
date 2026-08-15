/**
 * Internal statefile helpers shared between the engine (engine::dump/load) and the interactor
 * (the save_statefile/load_statefile commands).
 */

#ifndef f3d_statefile_h
#define f3d_statefile_h

#include <string>

namespace f3d
{
class options;
class scene;
class window;

namespace detail
{
/**
 * Capture the state of the given scene, window and options into a statefile JSON string.
 * The camera and window geometry are only captured when the window is not of type NONE.
 * File paths are stored as absolute paths, the canonical form of a state.
 */
std::string captureStateContent(const scene& scene, window& window, const options& options);

/**
 * Restore a statefile JSON string into the given scene, window and options, clearing the scene
 * first so the state fully replaces the current one.
 * Throws a f3d::engine::statefile_exception if the content cannot be parsed.
 */
void restoreStateContent(
  scene& scene, window& window, options& options, const std::string& content);
}
}

#endif

#ifndef f3d_engine_h
#define f3d_engine_h

#include "context.h"
#include "exception.h"
#include "export.h"
#include "interactor.h"
#include "options.h"
#include "scene.h"
#include "window.h"

#include <map>
#include <string>
#include <vector>

namespace f3d
{
/**
 * @class   engine
 * @brief   Class used to create instance of other classes
 *
 * The main class of the libf3d to create all other instances.
 * Configured on creation using an enum, then all objects
 * can be accessed through their getter.
 *
 * Example usage for adding some files in the scene
 *
 * \code{.cpp}
 *  f3d::engine eng = f3d::engine::create();
 *  f3d::scene& sce = eng.getscene();
 *  sce.add({"path/to/file", "path/to/another/file"});
 *  f3d::interactor& inter = eng.getInteractor();
 *  inter.start();
 * \endcode
 */
class F3D_EXPORT engine
{
public:
  /**
   * Create an engine with an automatic window.
   * Optionally, the window can be hidden by setting offscreen to true.
   * For VTK < 9.3, the window type will depend on the VTK build options
   * For VTK >= 9.4:
   * Linux: Try GLX, then EGL, then OSMesa
   * Windows: Try Win32, then EGL, then OSMesa
   * macOS: Always use Cocoa
   * Throws engine::no_window_exception in case of window creation failure.
   */
  static engine create(bool offscreen = false);

  /**
   * Create an engine with no window.
   */
  static engine createNone();

  /**
   * Create an engine with a GLX window.
   * Works on Linux only.
   * VTK >= 9.4 required.
   * Optionally, the window can be hidden by setting offscreen to true.
   * Throws engine::no_window_exception in case of window creation failure.
   */
  static engine createGLX(bool offscreen = false);

  /**
   * Create an engine with a WGL window.
   * Works on Windows only.
   * VTK >= 9.4 required.
   * Optionally, the window can be hidden by setting offscreen to true.
   * Throws engine::no_window_exception in case of window creation failure.
   */
  static engine createWGL(bool offscreen = false);

  /**
   * Create an engine with an EGL window.
   * VTK >= 9.4 required.
   * If several GPU are available, the environment variable
   * `VTK_DEFAULT_EGL_DEVICE_INDEX` allows its selection.
   * Optionally, the window can be hidden by setting offscreen to true.
   * Throws engine::no_window_exception in case of failure.
   */
  static engine createEGL(bool offscreen = false);

  /**
   * Create an engine with an OSMesa window.
   * VTK >= 9.4 required.
   */
  static engine createOSMesa();

  /**
   * Create an engine with an external window.
   * A context to retrieve OpenGL symbols is required.
   * The context can be nullptr for an external Cocoa window.
   * Here's an example if a GLFW window is used:
   * \code{.cpp}
   *  f3d::engine eng = f3d::engine::createExternal(glfwGetProcAddress);
   * \endcode
   */
  static engine createExternal(const context::function& getProcAddress);

  /**
   * Create an engine with an external GLX context.
   * Equivalent to createExternal(f3d::context::glx());
   * VTK >= 9.4 required.
   * Throws context::loading_exception if GLX library is not found or if not running on Linux.
   */
  static engine createExternalGLX();

  /**
   * Create an engine with an external WGL context.
   * Equivalent to createExternal(f3d::context::wgl());
   * VTK >= 9.4 required.
   * Throws context::loading_exception if WGL library is not found or if not running on Windows.
   */
  static engine createExternalWGL();

  /**
   * Create an engine with an external COCOA context.
   * Equivalent to createExternal(f3d::context::cocoa());
   * VTK >= 9.4 required.
   * Throws context::loading_exception if WGL library is not found or if not running on Windows.
   */
  static engine createExternalCOCOA();

  /**
   * Create an engine with an external EGL context.
   * Equivalent to createExternal(f3d::context::egl());
   * VTK >= 9.4 required.
   * Throws context::loading_exception if EGL library is not found.
   */
  static engine createExternalEGL();

  /**
   * Create an engine with an external OSMesa context.
   * Equivalent to createExternal(f3d::context::osmesa());
   * VTK >= 9.4 required.
   * Throws context::loading_exception if OSMesa library is not found.
   */
  static engine createExternalOSMesa();

  /**
   * Engine destructor, delete all object instances as well.
   */
  ~engine();

  //@{
  /**
   * Engine copy is not possible but move is allowed.
   */
  engine(const engine& other) = delete;
  engine(engine&& other) noexcept;
  engine& operator=(const engine& other) = delete;
  engine& operator=(engine&& other) noexcept;
  //@}

  /**
   * Set the cache path. Must be an absolute path.
   * Currently, it's only used to store HDRI baked textures.
   * By default, the cache path is:
   * - Windows: %LOCALAPPDATA%\f3d
   * - Linux: ~/.cache/f3d
   * - macOS: ~/Library/Caches/f3d
   */
  void setCachePath(const std::string& cachePath);

  /**
   * Engine provide a default options that you can use using engine::getOptions().
   * But you can use this setter to use other options directly.
   * It will copy options into engine.
   */
  engine& setOptions(const options& opt);

  /**
   * Engine provide a default options that you can use using engine::getOptions().
   * But you can use this setter to use other options directly.
   * It will move options into engine.
   */
  engine& setOptions(options&& opt);

  /**
   * Get the default options provided by the engine.
   */
  options& getOptions();

  /**
   * Get the window provided by the engine, if any.
   * If not, will throw a engine::no_window_exception.
   */
  window& getWindow();

  /**
   * Get the loaded provided by the engine.
   */
  scene& getScene();

  /**
   * Get the interactor provided by the engine, if any.
   * If not, will throw a engine::no_interactor_exception.
   */
  interactor& getInteractor();

  /**
   * Load a plugin.
   * Supports full path, relative path, and plugin name.
   * First try to load the plugin by name from the static plugins.
   * Then try to load the path provided as if it is a full path to a plugin.
   * Then try to load a plugin by its name looking into the provided plugin search paths.
   * Then try to load a plugin by its name relying on internal system (eg: LD_LIBRARY_PATH).
   * The plugin "native" is always available and includes native VTK readers.
   * If built and available in your build, F3D is providing 5 additional plugins:
   * "alembic", "assimp", "draco", "exodus", "occt", "usd".
   * Custom plugins can also be available that F3D is not supporting officially.
   * Throw a plugin_exception if the plugin can't be loaded for some reason.
   */
  static void loadPlugin(
    const std::string& nameOrPath, const std::vector<std::string>& pluginSearchPaths = {});

  /**
   * Automatically load all the static plugins.
   * The plugin "native" is guaranteed to be static.
   */
  static void autoloadPlugins();

  /**
   * List plugins based on associated json files located in the given directory.
   * Listed plugins can be loaded using engine::loadPlugin function.
   * Note that the listed plugins may fail to load if the library is not found or incompatible.
   */
  static std::vector<std::string> getPluginsList(const std::string& pluginPath);

  /**
   * A structure providing information about the libf3d.
   * Returned by getLibInfo().
   */
  struct libInformation
  {
    std::string Version;
    std::string VersionFull;
    std::string BuildDate;
    std::string BuildSystem;
    std::string Compiler;
    std::map<std::string, bool> Modules;
    std::string VTKVersion;
    std::vector<std::string> Copyrights;
    std::string License;
  };

  /**
   * Get a struct containing info about the libf3d.
   */
  static libInformation getLibInfo();

  /**
   * A structure providing information about a reader.
   * Returned in a vector by getReadersInfo().
   */
  struct readerInformation
  {
    std::string Name;
    std::string Description;
    std::vector<std::string> Extensions;
    std::vector<std::string> MimeTypes;
    std::string PluginName;
    bool HasSceneReader;
    bool HasGeometryReader;
  };

  /**
   * Get a vector of struct containing info about the supported readers.
   */
  static std::vector<readerInformation> getReadersInfo();

  /**
   * An exception that can be thrown by the engine
   * when no window is available.
   */
  struct no_window_exception : public exception
  {
    explicit no_window_exception(const std::string& what = "");
  };

  /**
   * An exception that can be thrown by the engine
   * when no interactor is available .
   */
  struct no_interactor_exception : public exception
  {
    explicit no_interactor_exception(const std::string& what = "");
  };

  /**
   * An exception that can be thrown by the engine
   * when a plugin cannot be loaded.
   */
  struct plugin_exception : public exception
  {
    explicit plugin_exception(const std::string& what = "");
  };

private:
  class internals;
  internals* Internals;

  /**
   * Engine constructor. This is a private method.
   * The user must rely on factories to create the engine instance.
   */
  engine(const std::optional<window::Type>& windowType, bool offscreen,
    const context::function& loader = nullptr);
};
}

#endif

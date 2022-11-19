#ifndef f3d_engine_h
#define f3d_engine_h

#include "exception.h"
#include "export.h"
#include "interactor.h"
#include "loader.h"
#include "options.h"
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
 * Example usage:
 *
 * \code{.cpp}
 *  f3d::engine eng(f3d::engine::CREATE_WINDOW | f3d::engine::CREATE_INTERACTOR);
 *  f3d::loader& load = eng.getLoader();
 *  load.addFile("path/to/file");
 *  load.loadFile(f3d::loader::LoadFileEnum::LOAD_CURRENT);
 *  f3d::interactor& inter = eng.getInteractor();
 *  inter.start();
 * \endcode
 */
class F3D_EXPORT engine
{
public:
  /**
   * Engine constructor, choose the window type using the enum.
   * see window.h for details about the window.
   * When using window::Type::NONE, window and interactor will not be provided by the engine.
   * When using window::Type::EXTERNAL, interactor will not be provided by the engine.
   * All objects instances will be created on construction.
   * Default is window::Type::NATIVE.
   * Throw a no_window_exception when using a Using window::Type::EXTERNAL without the right cmake
   * option.
   */
  explicit engine(window::Type windowType = window::Type::NATIVE);

  /**
   * Engine destructor, delete all object instances as well.
   */
  ~engine();

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
  loader& getLoader();

  /**
   * Get the interactor provided by the engine, if any.
   * If not, will throw a engine::no_interactor_exception.
   */
  interactor& getInteractor();

  /**
   * Load a plugin.
   * Supports full path, relative path, and plugin name.
   * On Linux and macOS, uses LD_LIBRARY_PATH to find the plugin.
   * On Windows, the plugin should be located in the same folder as the executable.
   * The plugin "native" is always available and includes native VTK readers.
   * If built and available in your build, f3d is providing 4 additional plugins:
   * "exodus", "occt", "assimp", "alembic".
   * Custom plugins can also be available that f3d is not supporting officially.
   * Throw a plugin_exception if the plugin can't be loaded for some reason.
   */
  static void loadPlugin(const std::string& path);

  /**
   * Automatically load all the static plugins.
   * The plugin "native" is guaranteed to be static.
   */
  static void autoloadPlugins();

  /**
   * A structure providing information about the libf3d.
   * Returned by getLibInfo().
   */
  struct libInformation
  {
    std::string Version;
    std::string BuildDate;
    std::string BuildSystem;
    std::string Compiler;
    std::string RaytracingModule;
    std::string ExternalRenderingModule;
    std::string VTKVersion;
    std::string PreviousCopyright;
    std::string Copyright;
    std::string License;
    std::string Authors;
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
    no_window_exception(const std::string& what = "");
  };

  /**
   * An exception that can be thrown by the engine
   * when no interactor is available .
   */
  struct no_interactor_exception : public exception
  {
    no_interactor_exception(const std::string& what = "");
  };

  /**
   * An exception that can be thrown by the engine
   * when a plugin cannot be loaded.
   */
  struct plugin_exception : public exception
  {
    plugin_exception(const std::string& what = "");
  };

private:
  class internals;
  internals* Internals;
  engine(const engine& opt) = delete;
  engine(engine&& opt) = delete;
  engine& operator=(const engine& opt) = delete;
  engine& operator=(engine&& opt) = delete;
};
}

#endif

#ifndef f3d_engine_h
#define f3d_engine_h

#include "context.h"
#include "exception.h"
#include "export.h"
#include "interactor.h"
#include "options.h"
#include "scene.h"
#include "window.h"

/// @cond
#include <filesystem>
#include <iosfwd>
#include <map>
#include <string>
#include <vector>
/// @endcond

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
   * Linux: GLX if a X server is running, otherwise EGL if available, otherwise OSMesa.
   * Windows: Win32 if it supports OpenGL >= 3.2, otherwise EGL if available, otherwise OSMesa.
   * macOS: Always use Cocoa.
   *
   * Throws a context::loading_exception if a needed graphic library cannot be loaded.
   * Throws a context::symbol_exception if a needed symbol cannot be found in graphic library.
   * Throws a engine::no_window_exception if the window cannot be created for another reason.
   * Throws a engine::cache_exception if the default cache directory cannot be used.
   */
  [[nodiscard]] static engine create(bool offscreen = false);

  /**
   * Create an engine with no window.
   *
   * Throws a engine::no_window_exception if the window cannot be created for another reason.
   * Throws a engine::cache_exception if the default cache directory cannot be used.
   */
  [[nodiscard]] static engine createNone();

  /**
   * Create an engine with a GLX window.
   * Works on Linux only.
   * Optionally, the window can be hidden by setting offscreen to true.
   *
   * Throws a context::loading_exception if a needed graphic library cannot be loaded.
   * Throws a context::symbol_exception if a needed symbol cannot be found in graphic library.
   * Throws a engine::no_window_exception if the window cannot be created for another reason.
   * Throws a engine::cache_exception if the default cache directory cannot be used.
   */
  [[nodiscard]] static engine createGLX(bool offscreen = false);

  /**
   * Create an engine with a WGL window.
   * Works on Windows only.
   * Optionally, the window can be hidden by setting offscreen to true.
   *
   * Throws a context::loading_exception if a needed graphic library cannot be loaded.
   * Throws a context::symbol_exception if a needed symbol cannot be found in graphic library.
   * Throws a engine::no_window_exception if the window cannot be created for another reason.
   * Throws a engine::cache_exception if the default cache directory cannot be used.
   */
  [[nodiscard]] static engine createWGL(bool offscreen = false);

  /**
   * Create an engine with an offscreen EGL window.
   * If several GPU are available, the environment variable
   * `VTK_DEFAULT_EGL_DEVICE_INDEX` allows its selection.
   * Not supported on macOS.
   *
   * Throws a context::loading_exception if a needed graphic library cannot be loaded.
   * Throws a context::symbol_exception if a needed symbol cannot be found in graphic library.
   * Throws a engine::no_window_exception if the window cannot be created for another reason.
   * Throws a engine::cache_exception if the default cache directory cannot be used.
   */
  [[nodiscard]] static engine createEGL();

  /**
   * Create an engine with an offscreen OSMesa window.
   * On macOS, VTK >= 9.6.20260630 is required.
   *
   * Throws a context::loading_exception if a needed graphic library cannot be loaded.
   * Throws a context::symbol_exception if a needed symbol cannot be found in graphic library.
   * Throws a engine::no_window_exception if the window cannot be created for another reason.
   * Throws a engine::cache_exception if the default cache directory cannot be used.
   */
  [[nodiscard]] static engine createOSMesa();

  /**
   * Create an engine with a webassembly window.
   * The canvas element can be selected using the `canvasSelector` parameter.
   *
   * Throws a engine::no_window_exception if not using WebAssembly.
   */
  [[nodiscard]] static engine createWasm(std::string_view canvasSelector = "#canvas");

  /**
   * Create an engine with an external window.
   * A context to retrieve OpenGL symbols is required.
   * Here's an example if a GLFW window is used:
   * \code{.cpp}
   *  f3d::engine eng = f3d::engine::createExternal(glfwGetProcAddress);
   * \endcode
   *
   * Throws a engine::no_window_exception if the window cannot be created.
   * Throws a engine::cache_exception if the default cache directory cannot be used.
   */
  [[nodiscard]] static engine createExternal(const context::function& getProcAddress);

  /**
   * Create an engine with an external GLX context.
   * Equivalent to createExternal(f3d::context::glx());
   *
   * Throws a context::loading_exception if a needed graphic library cannot be loaded.
   * Throws a context::symbol_exception if a needed symbol cannot be found in graphic library.
   * Throws a engine::no_window_exception if the window cannot be created for another reason.
   * Throws a engine::cache_exception if the default cache directory cannot be used.
   */
  [[nodiscard]] static engine createExternalGLX();

  /**
   * Create an engine with an external WGL context.
   * Equivalent to createExternal(f3d::context::wgl());
   *
   * Throws a context::loading_exception if a needed graphic library cannot be loaded.
   * Throws a context::symbol_exception if a needed symbol cannot be found in graphic library.
   * Throws a engine::no_window_exception if the window cannot be created for another reason.
   * Throws a engine::cache_exception if the default cache directory cannot be used.
   */
  [[nodiscard]] static engine createExternalWGL();

  /**
   * Create an engine with an external COCOA context.
   * Equivalent to createExternal(f3d::context::cocoa());
   *
   * Throws a context::loading_exception if a needed graphic library cannot be loaded.
   * Throws a context::symbol_exception if a needed symbol cannot be found in graphic library.
   * Throws a engine::no_window_exception if the window cannot be created for another reason.
   * Throws a engine::cache_exception if the default cache directory cannot be used.
   */
  [[nodiscard]] static engine createExternalCOCOA();

  /**
   * Create an engine with an external EGL context.
   * Equivalent to createExternal(f3d::context::egl());
   *
   * Throws a context::loading_exception if a needed graphic library cannot be loaded.
   * Throws a context::symbol_exception if a needed symbol cannot be found in graphic library.
   * Throws a engine::no_window_exception if the window cannot be created for another reason.
   * Throws a engine::cache_exception if the default cache directory cannot be used.
   */
  [[nodiscard]] static engine createExternalEGL();

  /**
   * Create an engine with an external OSMesa context.
   * Equivalent to createExternal(f3d::context::osmesa());
   *
   * Throws a context::loading_exception if a needed graphic library cannot be loaded.
   * Throws a context::symbol_exception if a needed symbol cannot be found in graphic library.
   * Throws a engine::no_window_exception if the window cannot be created for another reason.
   * Throws a engine::cache_exception if the default cache directory cannot be used.
   */
  [[nodiscard]] static engine createExternalOSMesa();

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
   * Set the cache path. The provided path is used as is.
   * Currently, it's only used to store HDRI baked textures.
   * By default, the cache path is:
   * - Windows: %LOCALAPPDATA%\f3d
   * - Linux: ~/.cache/f3d
   * - macOS: ~/Library/Caches/f3d
   *
   * Throws a engine::cache_exception if the provided cachePath cannot be used.
   */
  engine& setCachePath(const std::filesystem::path& cachePath);

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
  [[nodiscard]] options& getOptions();

  /**
   * Get the window provided by the engine, if any.
   * If not, will throw a engine::no_window_exception.
   */
  [[nodiscard]] window& getWindow();

  /**
   * Get the loaded provided by the engine.
   */
  [[nodiscard]] scene& getScene();

  /**
   * Get the interactor provided by the engine, if any.
   * If not, will throw a engine::no_interactor_exception.
   */
  [[nodiscard]] interactor& getInteractor();

  /**
   * A serializable snapshot of an engine: the files added to the scene, the camera configuration
   * and the options that have been set. Captured from an engine with engine::dump() and applied
   * back with engine::load(), so a state taken from one engine can be restored into another.
   *
   * A state can be read from and written to a JSON string, a file, the system clipboard or any
   * stream. In its string, stream and clipboard forms, file paths are stored as absolute paths; in
   * its file form, file paths contained by the directory of the file are stored relatively so the
   * statefile stays portable when moved alongside its files.
   *
   * Scene content added from memory (mesh, mesh view or buffer) is not captured.
   */
  struct state
  {
    /**
     * Build a state from a JSON string previously produced by state::toString.
     * Throws a engine::statefile_exception if the content cannot be parsed.
     */
    [[nodiscard]] static state fromString(const std::string& content);

    /**
     * Build a state from a JSON statefile previously written by state::toFile.
     * File paths stored relatively are resolved against the directory of the file.
     * Throws a engine::statefile_exception if the file cannot be read or parsed.
     */
    [[nodiscard]] static state fromFile(const std::filesystem::path& filePath);

    /**
     * Build a state from the JSON content currently held by the system clipboard.
     * Throws a engine::statefile_exception if the clipboard has no readable text, its content
     * cannot be parsed, or clipboard support is not available in this build.
     */
    [[nodiscard]] static state pasteClipboard();

    /**
     * Return the state as a JSON string, with file paths stored as absolute paths.
     */
    [[nodiscard]] std::string toString() const;

    /**
     * Write the state as a JSON statefile at the provided path. File paths contained by the
     * directory of the file are stored relatively, as absolute paths otherwise.
     * Throws a engine::statefile_exception if the file cannot be written.
     */
    void toFile(const std::filesystem::path& filePath) const;

    /**
     * Copy the state into the system clipboard as a JSON string, with file paths stored as absolute
     * paths.
     * Throws a engine::statefile_exception if the clipboard cannot be written or clipboard support
     * is not available in this build.
     */
    void copyClipboard() const;

  private:
    friend class engine;
    friend std::ostream& operator<<(std::ostream& stream, const state& st);
    friend std::istream& operator>>(std::istream& stream, state& st);

    std::string Content;
  };

  /**
   * Capture the current state of the engine (added files, camera and options) into a state that can
   * be serialized and later applied back with engine::load.
   * Scene content added from memory (mesh, mesh view or buffer) is not captured.
   */
  [[nodiscard]] state dump();

  /**
   * Restore the engine from a previously captured state. The scene is cleared first, then the saved
   * files are added, the options are set and the camera configuration is restored.
   * Throws a engine::statefile_exception if the state content cannot be parsed.
   * Throws a scene::load_failure_exception if one of the saved files cannot be loaded.
   */
  engine& load(const state& st);

  /**
   * List rendering backends supported by libf3d.
   * All backends have an associated boolean flag indicating if it can be used.
   */
  static std::map<std::string, bool> getRenderingBackendList();

  /**
   * Load a plugin.
   * The provided pathOrName can be a full path, relative path, or plugin name.
   * First try to load the plugin by name from the static plugins.
   * Then try to load the path provided as if it is a full path to a plugin.
   * Then try to load a plugin by its name looking into the provided plugin search paths (used as
   * is). Then try to load a plugin by its name relying on internal system (eg: LD_LIBRARY_PATH).
   *
   * The plugin "native" is always available and includes native VTK readers.
   * If built and available in your build, F3D is providing 6 additional plugins:
   * "alembic", "assimp", "draco", "hdf", "occt", "usd", "vdb".
   *
   * Custom plugins can also be available that F3D is not supporting officially.
   * Throws a engine::plugin_exception if the plugin can't be loaded for some reason.
   */
  static void loadPlugin(const std::string& pathOrName,
    const std::vector<std::filesystem::path>& pluginSearchPaths = {});

  /**
   * Automatically load all the static plugins.
   * The plugin "native" is guaranteed to be static.
   */
  static void autoloadPlugins();

  /**
   * List plugins based on associated json files located in the given directory, used as is.
   * Listed plugins can be loaded using engine::loadPlugin function.
   * Note that the listed plugins may fail to load if the library is not found or incompatible.
   *
   * Return available plugins if any, or an empty vector if there are none or the provided path does
   * not exist.
   */
  [[nodiscard]] static std::vector<std::string> getPluginsList(
    const std::filesystem::path& pluginPath);

  /**
   * Get all plugin option names that can be set using `setReaderOption`
   * This vector can be expanded when loading plugin using `loadPlugin`
   */
  [[nodiscard]] static std::vector<std::string> getAllReaderOptionNames();

  /**
   * Set a specific reader option from a plugin to provided value if it exists.
   * Throws a options::inexistent_exception if the option does not exists in any reader of any
   * plugin.
   */
  static void setReaderOption(const std::string& name, const std::string& value);

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
  [[nodiscard]] static libInformation getLibInfo();

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
    bool SupportsStream;
  };

  /**
   * Get a vector of struct containing info about the supported readers.
   */
  [[nodiscard]] static std::vector<readerInformation> getReadersInfo();

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

  /**
   * An exception that can be thrown by the engine
   * when the cache cannot be used
   */
  struct cache_exception : public exception
  {
    explicit cache_exception(const std::string& what = "");
  };

  /**
   * An exception that can be thrown by the engine
   * when a statefile cannot be read, written or parsed.
   */
  struct statefile_exception : public exception
  {
    explicit statefile_exception(const std::string& what = "");
  };

private:
  class internals;
  internals* Internals;

  /**
   * Engine constructor. This is a private method.
   * The user must rely on factories to create the engine instance.
   */
  engine(const std::optional<window::Type>& windowType, bool offscreen,
    const context::function& loader, std::string_view id = "");
};

/**
 * Write a state as a JSON string into the provided stream, with file paths stored as absolute
 * paths.
 */
F3D_EXPORT std::ostream& operator<<(std::ostream& stream, const engine::state& st);

/**
 * Read a state from the JSON content of the provided stream.
 * Throws a engine::statefile_exception if the content cannot be parsed.
 */
F3D_EXPORT std::istream& operator>>(std::istream& stream, engine::state& st);
}

#endif

/**
 * @class   engine
 * @brief   Class used to create instance of other classes
 *
 * The main class of the libf3d to create all other instances
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

#ifndef f3d_engine_h
#define f3d_engine_h

#include "export.h"
#include "window.h"

#include <map>
#include <stdexcept>
#include <string>
#include <vector>

namespace f3d
{
class options;
class loader;
class interactor;
class F3D_EXPORT engine
{
public:
  class exception : public std::runtime_error
  {
  public:
    exception(const std::string& what = "")
      : std::runtime_error(what)
    {
    }
  };

  /**
   * Engine constructor, choose the window type using the enum.
   * see window.h for details about the window.
   * When using window::Type::NONE, window and interactor will not be provided by the engine.
   * When using window::Type::EXTERNAL, interactor will not be provided by the engine.
   * All objects instances will be created on construction
   */
  explicit engine(window::Type windowType = window::Type::NATIVE);

  /**
   * Engine destructor, delete all object instances as well
   */
  ~engine();

  /**
   * Engine provide a default options that you can use using engine::getOptions()
   * But you can use this setter to use other options directly.
   * It will copy options into engine
   */
  void setOptions(const options& opt);

  /**
   * Engine provide a default options that you can use using engine::getOptions()
   * But you can use this setter to use other options directly.
   * It will move options into engine
   */
  void setOptions(options&& opt);

  /**
   * Get the default options provided by the engine
   */
  options& getOptions();

  /**
   * Get the window provided by the engine, if any.
   * If not, will throw a engine::exception
   */
  window& getWindow();

  /**
   * Get the loaded provided by the engine
   */
  loader& getLoader();

  /**
   * Get the interactor provided by the engine, if any
   * If not, will throw a engine::exception
   */
  interactor& getInteractor();

  /**
   * Get a map containing info about the libf3d
   * TODO improve this doc to list the keys
   */
  static std::map<std::string, std::string> getLibInfo();

  struct readerInformation
  {
    std::string name;
    std::string description;
    std::vector<std::string> extensions;
    std::vector<std::string> mimetypes;
  };

  /**
   * Get a vector containing info about the supported readers
   */
  static std::vector<readerInformation> getReadersInfo();

private:
  class internals;
  internals* Internals;
  engine(const engine& opt) = delete;
  engine& operator=(const engine& opt) = delete;
};
}

#endif

/**
 * @class   factory
 * @brief   The factory that manages the reader class instances
 *
 * This class describes the singleton object that manages the declared readers
 * in F3D.
 * To make it simple and automatic, you just have to declare each reader class
 * with the CMake macro f3d_declare_reader(). Then, at configure time, CMake
 * generates a cxx file that instantiates all the reader classes and registers
 * them to the factory.
 */

#ifndef f3d_plugin_factory_h
#define f3d_plugin_factory_h

#include "plugin.h"
#include "reader.h"

#include <map>
#include <vector>

namespace f3d
{
class factory
{
public:
  using plugin_initializer_t = plugin* (*)();

  /**
   * Get instance
   */
  static factory* instance();

  /**
   * Register a plugin to the factory
   */
  void load(plugin*);

  /**
   * Register all static plugins to the factory
   */
  void autoload();

  /**
   * Get the reader that can read the given file, nullptr if none
   */
  reader* getReader(const std::string& fileName);

  /**
   * Get the list of the registered plugins
   */
  const std::vector<plugin*>& getPlugins();

  /**
   * Get static plugin initialization function
   * Return nullptr if it does not exists
   */
  plugin_initializer_t getStaticInitializer(const std::string& pluginName);

protected:
  factory();
  virtual ~factory() = default;

  bool registerOnce(plugin* p);

  std::vector<plugin*> Plugins;

  std::map<std::string, plugin_initializer_t> StaticPluginInitializers;
};
}
#endif

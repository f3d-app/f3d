/**
 * @class   plugin
 * @brief   The base class of plugins
 *
 * Plugins implementation are automatically generated with CMake macros.
 * This is the base class of every plugin.
 *
 * See ``f3d_plugin_init``, ``f3d_plugin_declare_reader``, and ``f3d_plugin_build`` in
 * the ``f3dPlugin.cmake`` module.
 */

#ifndef f3d_plugin_h
#define f3d_plugin_h

#include "reader.h"

#include <memory>
#include <string>
#include <vector>

namespace f3d
{
class plugin
{
public:
  plugin(const std::string& name, const std::string& desc, const std::string& vers,
    const std::vector<std::shared_ptr<reader> >& readers)
    : Name(name)
    , Description(desc)
    , Version(vers)
    , Readers(readers)
  {
  }

  const std::string& getName() { return Name; }
  const std::string& getDescription() { return Description; }
  const std::string& getVersion() { return Version; }

  const std::vector<std::shared_ptr<reader> >& getReaders() { return Readers; }

private:
  std::string Name;
  std::string Description;
  std::string Version;
  std::vector<std::shared_ptr<reader> > Readers;
};

}

#endif

#ifndef f3d_config_h
#define f3d_config_h

#include <string>

namespace f3d
{
// TODO all of this should be prefixed Lib
// Or move to the app dir accordingly
extern const std::string AppName;
extern const std::string AppTitle;
extern const std::string LibVersion;
extern const std::string LibBuildSystem;
extern const std::string LibBuildDate;
extern const std::string LibCompiler;
extern const bool HasRaytracingModule;
}
#endif

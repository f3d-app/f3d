#ifndef f3d_config_h
#define f3d_config_h

#include <string>

namespace f3d
{
// TODO all of this should be prefixed Lib
// Or move to the app dir accordingly
extern const std::string AppName;
extern const std::string AppTitle;
extern const std::string AppVersion;
extern const std::string AppBuildSystem;
extern const std::string AppBuildDate;
extern const std::string AppCompiler;
extern const std::string ReservedString; // TODO to remove once not necessary
}
#endif

/**
 * @class   F3DSystemTools
 * @brief   A namespace to recover system path, cross platform
 *
 */

#ifndef F3DSystemTools_h
#define F3DSystemTools_h

#include <filesystem>

namespace F3DSystemTools
{
std::filesystem::path GetApplicationPath();
}

#endif

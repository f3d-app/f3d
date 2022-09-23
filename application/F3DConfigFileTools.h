/**
 * @class   F3DConfigFileTools
 * @brief   A namespace to recover path to config file and related directories, cross platform
 *
 */

#ifndef F3DTools_h
#define F3DTools_h

#include <string>

namespace F3DConfigFileTools
{
std::string GetUserConfigFileDirectory();
std::string GetBinaryConfigFileDirectory();
std::string GetSystemConfigFileDirectory();
std::string GetConfigFilePath(const std::string& filename);
}

#endif

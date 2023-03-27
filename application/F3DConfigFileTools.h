/**
 * @class   F3DConfigFileTools
 * @brief   A namespace to recover path to config file and related directories, cross platform
 *
 */

#ifndef F3DTools_h
#define F3DTools_h

#include <filesystem>
#include <string>

namespace F3DConfigFileTools
{
std::filesystem::path GetUserConfigFileDirectory();
std::filesystem::path GetBinaryConfigFileDirectory();
std::filesystem::path GetSystemConfigFileDirectory();
std::filesystem::path GetConfigFilePath(const std::string& configSearch);
}

#endif

/**
 * @class   F3DConfigFileTools
 * @brief   A namespace to recover path to config file and related directories, cross platform
 *
 */

#ifndef F3DConfigFileTools_h
#define F3DConfigFileTools_h

#include <filesystem>
#include <string>

namespace F3DConfigFileTools
{
std::filesystem::path GetUserConfigFileDirectory();
std::filesystem::path GetBinaryConfigFileDirectory();
std::filesystem::path GetSystemConfigFileDirectory();
std::filesystem::path GetConfigPath(const std::string& configSearch);
}

#endif

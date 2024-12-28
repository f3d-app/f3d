/**
 * @class   F3DColorMapReader
 * @brief   A namespace used to convert images to libf3d colormap option
 *
 */

#ifndef F3DColorMapReader_h
#define F3DColorMapReader_h

#include <filesystem>
#include <string>
#include <vector>

namespace F3DColorMapTools
{
std::filesystem::path Find(const std::string& str);
std::vector<double> Read(const std::string& path);
}

#endif

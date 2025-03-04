#ifndef F3DConfigFileTools_h
#define F3DConfigFileTools_h
/**
 * @class   F3DConfigFileTools
 * @brief   A namespace to parse config files
 */
#include "F3DOptionsTools.h"

#include <string>

namespace F3DConfigFileTools
{
using BindingsVector = std::vector<std::pair<std::string, std::vector<std::string>>>;
using BindingsEntry = std::tuple<BindingsVector, std::string, std::string>;
using BindingsEntries = std::vector<BindingsEntry>;

/**
 * Read config files using userConfig if any, return an optionEntries
 * containing ordered optionDict and ordered bindingsEntries
 */
std::tuple<F3DOptionsTools::OptionsEntries, F3DOptionsTools::OptionsEntries, BindingsEntries>
ReadConfigFiles(const std::string& userConfig);
}

#endif

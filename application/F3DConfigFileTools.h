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

struct ParsedConfigFiles
{
  F3DOptionsTools::OptionsEntries Options;
  F3DOptionsTools::OptionsEntries ImperativeOptions;
  BindingsEntries Bindings;
};

/**
 * Read config files using userConfig if any, return a ParsedConfigFiles
 * containing ordered optionDict, ordered imperative optionDict and ordered bindingsEntries
 */
ParsedConfigFiles ReadConfigFiles(const std::string& userConfig);
}

#endif

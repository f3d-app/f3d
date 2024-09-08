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
/**
 * Read config files using userConfig if any, return an optionEntries
 * containing ordered optionDict.
 */
F3DOptionsTools::OptionsEntries ReadConfigFiles(const std::string& userConfig);
}

#endif
